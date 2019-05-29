/*
 * main.cpp
 *
 *  Created on: Sep 4, 2017
 *      Author: pete
 */

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <unistd.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "MessageBroker.h"
#include "geiger/PocketGeiger.h"

PocketGeiger *g_pg;
MessageBroker *g_broker;
bool g_mqttAvailable;

void usage(const char *name)
{
    std::cerr << "usage: " << name << " -l <pin:pin> -d" << std::endl;
    std::cerr << "\t-l <pin:pin> Enable Pocket Geiger counter with signal pin:noise pin" << std::endl;
    std::cerr << "\tNote, pins must be numeric and colon seperated to work" << std::endl;
    std::cerr << "\t-d Daemonize the application to run in the background" << std::endl;
    exit(-1);
}

void runtime()
{
    while (1) {
        if(g_pg)
            g_pg->loop();

        sleep(1);
    }
}

/**
 * \func void subscribe_to_topics(std::string &name)
 * \param name std::string name identifier for MQTT subscriptions for this device
 * This function will call the MessageBroker subscribe method to subscribe to the
 *  named topics
 */
void subscribe_to_topics(std::string &name)
{
	std::string topic = "weather/";

	if (!g_mqttAvailable)
		return;

	g_broker->subscribeTopic(topic + "hello");
	g_broker->subscribeTopic(topic + "weather/sensor/#");
}

/**
 * \func void send_hello_data()
 * This function is a callback assigned to MessageBroker which will send
 * a replay to a hello MQTT message with the state of all currently
 * enable relays.
 */
void send_hello_data()
{
	std::cout << __PRETTY_FUNCTION__ << ": reporting available sensors" << std::endl;
}

int split_args(const char *name, const char *input, std::vector<int> &result)
{
    std::istringstream f(input);
    std::string s;    
    while (getline(f, s, ':')) {
        try {
        	result.push_back(std::stoi(s));
        }
        catch (std::exception &e) {
        	std::cerr << e.what() << std::endl;
        	usage(name);
        }
    }
    return result.size();
}

/**
 * \func void get_name(std::string &name)
 * \param name C++ reference to std::string
 * This function attempts to get the kernel hostname for this device and assigns it to name.
 */
void get_name(std::string &name)
{
	std::ifstream ifs;
	int pos;

	ifs.open("/proc/sys/kernel/hostname");
	if (!ifs) {
		std::cerr << __PRETTY_FUNCTION__ << ": Unable to open /proc/sys/kernel/hostname for reading" << std::endl;
		name = "omega";
	}
	name.assign((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
	try {
		name.erase(name.find('\n'));
	}
	catch (std::out_of_range &e) {
		std::cerr << __PRETTY_FUNCTION__ << ": " << e.what() << std::endl;
		return;
	}
	std::cout << __PRETTY_FUNCTION__ << ": Assigning " << name << " as device name" << std::endl;
}

/**
 * \func bool parse_args(int argc, char **argv, std::string &name, std::string &mqtt, int &port)
 * \param arc integer argument count provied by shell
 * \param argv char ** array of arguments provided by shell
 * \param name std::string name of device which would be overridden with -n
 * \param mqtt std::string hostname or IP of Mosquitto server to connect to
 * \param port integer port number to override default Mosquitto server port number
 * Use getopt to set runtime arguments used by this server
 */
bool parse_args(int argc, char **argv, std::string &name, std::string &mqtt, int &port, bool &daemonize)
{
	int opt;
	bool rval = true;
    std::vector<int> args;

	if (argv) {
		while ((opt = getopt(argc, argv, "dl:m:p:n:")) != -1) {
			switch (opt) {
			case 'm':
				mqtt.clear();
				mqtt = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 'n':
				name = optarg;
				break;
	        case 'l':
	            if (split_args(argv[0], optarg, args) == 2)
	                g_pg = new PocketGeiger(args.at(0), args.at(1));
	            else
	                usage(argv[0]);
	            break;
	        case 'd':
	            daemonize = true;
	            break;
	        default:
	            usage(argv[0]);
			}
		}
	}

	return rval;
}

void send_sensor_data(std::string payload)
{
	std::cout << __PRETTY_FUNCTION__ << ": " << payload << std::endl;
}

void send_all_data()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}

int main(int argc, char *argv[])
{
	pid_t pid;
	std::string name;
	std::string mqtt;
	std::mutex startlock;
    g_pg = nullptr;
    bool daemonize = false;
    int port;
    int startCount = 0;

    parse_args(argc, argv, name, mqtt, port, daemonize);

	if (mqtt.length() != 0) {
		g_mqttAvailable = true;
		g_broker = new MessageBroker(name.c_str(), mqtt.c_str(), port);
		g_broker->setMutex(&startlock);
		g_broker->setHelloCallback(send_hello_data);
		g_broker->setSensorDataCallback(send_sensor_data);
		g_broker->setAllDataCallback(send_all_data);

		while (!startlock.try_lock()) {
			if (startCount++ > 30) {
				std::cerr << __PRETTY_FUNCTION__ << ": Unable to get MQTT connected notification, bailiing out" << std::endl;
				exit(2);
			}
			sleep(1);
		}
	}
	else {
		std::cout << __PRETTY_FUNCTION__ << ": No MQTT server address provided, running dark..." << std::endl;
		g_mqttAvailable = false;
	}

	subscribe_to_topics(name);

    if (daemonize) {
    	pid = fork();
    	if (pid == 0) {
    		runtime();
    	}
    	else if (pid < 0) {
    		std::cerr << "fork failed";
    	}
    	else {
    		fclose(stdout);
    		fclose(stdin);
    		fclose(stderr);
    	}
    }
    else {
        runtime();
    }

	exit(0);
}
