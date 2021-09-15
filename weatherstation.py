import time
import board
import busio
import json
import threading
import adafruit_veml7700
import adafruit_veml6070
import RPi.GPIO as GPIO
from RPi_AS3935_SPI import RPi_AS3935
from datetime import datetime
from suntime import Sun, SunTimeException
import paho.mqtt.client as mqtt
from ButtonHandler import ButtonHandler
import adafruit_sht31d
from PiPocketGeiger import RadiationWatch

latitude = 42.0884
longitude = -87.9806
lightningint = 5
rainint = 19
rainfall = 0
rainfallytd = 0
updaterainfall = False
mutex = threading.Lock()

# ≥ 11 ≥ 2055 Extreme
# 8 to 10 1494 to 2054 Very High
# 6, 7 1121 to 1494 High
# 3 to 5 561 to 1120 Moderate
# 0 to 2 0 to 560

def uv_index(raw):
    if raw <= 230:
        return 1
    if raw <= 560:
        return 2
    if raw <= 747:
        return 3
    if raw <= 934:
        return 4
    if raw <= 1120:
        return 5
    if raw <= 1307:
        return 6
    if raw <= 1494:
        return 7
    if raw <= 1681:
        return 8
    if raw <= 1867:
        return 9
    if raw <= 2054:
        return 10
    
    return 11

def scale_humidity(humidity):
    half = 100 - humidity
    if humidity <= 50:
        half = humidity
        
    return int(half / 15)
                
def drop_noisefloor():
    floor = as3935.get_noise_floor()
    
    if floor == 0:
        return
    
    if floor > 1:
        as3935.set_noise_floor(floor - 1)
        threading.Timer(3600, drop_noisefloor).start()
    else:
        as3935.set_noise_floor(0)
        
# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        client.connected_flag = True
    else:
        print("Connection failed! " + str(rc))

def lightning_interrupt(channel):
    time.sleep(0.003)
    global as3935
    reason = as3935.get_interrupt()
    if reason == 0x01:
        print("Noise level too high - adjusting")
        as3935.raise_noise_floor()
        threading.Timer(3600, drop_noisefloor).start()
    elif reason == 0x04:
        print ("Disturber detected - masking")
        as3935.set_mask_disturber(True)
    elif reason == 0x08:
        now = datetime.now()
        parent = {}
        lightning = {}
        lightning["distance"] = as3935.get_distance()
        lightning["noisefloor"] = as3935.get_noise_floor()
        parent["timestamp"] = now.strftime("%d/%m/%Y %H:%M:%S")
        parent["lightning"] = lightning
        result = json.dumps(parent)
        client.publish("weather/lightning", result)

def rain_interrupt(args):
    global rainfall, rainfallytd, updaterainfall
    
    mutex.acquire()
    rainfall += 1
    rainfallytd += 1
    updaterainfall = True
    mutex.release()
    
def read_rainfall():
    global rainfall, rainfallytd

    try:
        f = open("rainfall.txt", "r")
    except OSError:
        print ("rainfall.txt doesn't exist, values will be 0")
        return
    
    try:
        data = json.load(f)
    except ValueError:
        print ("JSON decode error in rainfall.txt, values will be 0")
        return
    
    rainfall = data["today"]
    rainfallytd = data["ytd"]
    print (data)
    f.close()
    
# This is a thread that sits and waits in the background for events
# if rain is detected this will store it
def write_rainfall():
    global rainfall, rainfallytd, updaterainfall
    
    while True:
        if updaterainfall == True:
            f = open("rainfall.txt", "w")
            parent = {}
            mutex.acquire()
            parent["today"] = rainfall
            parent["ytd"] = rainfallytd
            f.write(json.dumps(parent))
            updaterainfall = False
            mutex.release()
            f.close()
            
        time.sleep(10)
    
mqtt.Client.connected_flag = False
client = mqtt.Client("weatherstation")
client.on_connect = on_connect
#as3935 = RPi_AS3935(bus=0, address=1, mode=0b10, speed=400000)
i2c = busio.I2C(board.SCL, board.SDA)
veml7700 = adafruit_veml7700.VEML7700(i2c)
uv = adafruit_veml6070.VEML6070(i2c)
sun = Sun(latitude, longitude)
sht31 = adafruit_sht31d.SHT31D(i2c)
radiationWatch = RadiationWatch(24, 23)

writerthread = threading.Thread(target=write_rainfall)

GPIO.setmode(GPIO.BCM)
GPIO.setup(lightningint, GPIO.IN)
GPIO.setup(rainint, GPIO.IN, pull_up_down=GPIO.PUD_UP)
raincb = ButtonHandler(rainint, rain_interrupt, edge='falling', bouncetime=100)
GPIO.add_event_detect(lightningint, GPIO.RISING, callback=lightning_interrupt)
GPIO.add_event_detect(rainint, GPIO.FALLING, callback=raincb)

def json_message():
    now = datetime.now()
    humidity = sht31.relative_humidity
    humidity -= scale_humidity(sht31.relative_humidity)
    
    json_parent = {}
    environment = {}
    rain = {}
    environment["uv"] = uv.uv_raw
    environment["uvindex"] = uv_index(raw=uv.uv_raw) 
    environment["lux"] = veml7700.light
    environment["temperature"] = sht31.temperature
    environment["humidity"] = humidity
    rain["today"] = rainfall
    rain["year"] = rainfallytd
    json_parent["timestamp"] = now.strftime("%d/%m/%Y %H:%M:%S")
    json_parent["environment"] = environment
    json_parent["rain"] = rain
    json_parent["radiation"] = radiationWatch.status()
    result = json.dumps(json_parent)
    client.publish("weather/conditions", result)
    threading.Timer(60, json_message).start()
    
def main():
#    as3935.set_indoors(True)
#    as3935.set_noise_floor(0)
#    as3935.calibrate(tun_cap=0x00)
    radiationWatch.setup()

    read_rainfall()

    raincb.start()
    writerthread.start()

    client.connect("172.24.1.12", 1883, 60)
    client.loop_start()
    while not client.connected_flag: #wait in loop
        time.sleep(1)
    
    json_message()
    
    while True:
        client.loop()
        time.sleep(1.0)

if __name__ == "__main__":
    main()

