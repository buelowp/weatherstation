/*
 * MessageBroker.cpp
 *
 *  Created on: Jan 16, 2019
 *      Author: Peter Buelow
 */

#include <MessageBroker.h>

MessageBroker::MessageBroker(const char *name, const char *host, int port) :
	m_host(host), m_name(name), m_port(port), m_connected(false), mosquittopp(name)
{
	m_mutex = nullptr;
	mosqpp::lib_init();			// Initialize libmosquitto

	std::cout << __PRETTY_FUNCTION__ << ": Connecting " << name << " to " << host << " at port " << port << std::endl;
	connect(m_host.c_str(), m_port, 120);
	loop_start();
}

MessageBroker::~MessageBroker()
{
	loop_stop();
	mosqpp::lib_cleanup();
}

void MessageBroker::setMutex(std::mutex *m)
{
	if (m != nullptr) {
		m_mutex = m;
		if (m_connected) {
			m_mutex->unlock();
		}
		else
			m_mutex->lock();
	}
}

void MessageBroker::on_connect(int rc)
{
	std::cout << __PRETTY_FUNCTION__ << ": Connection code " << rc << std::endl;
	if (rc == 0) {
		std::cout << __PRETTY_FUNCTION__ << ": Connected to mosquitto server " << m_host << std::endl;
		m_connected = true;
	}
	else {
		std::cerr << __PRETTY_FUNCTION__ << ": Connection failed with reason code " << rc << std::endl;
	}
	if (m_mutex) {
		m_mutex->unlock();
	}
}

void MessageBroker::on_disconnect(int rc)
{
	std::cout << __PRETTY_FUNCTION__ << ": We have been cut off with reason code " << rc << "..." << std::endl;
	m_connected = false;
	loop_stop();
	connect_async(m_host.c_str(), m_port, 120);
	loop_start();
}

void MessageBroker::on_message(const struct mosquitto_message *msg)
{
	std::string topic = msg->topic;
	std::string payload;
	std::string alldata = "weather/sensor/all";
	std::string sensordata = "weather/sensor";

	if (msg->payloadlen) {
		payload = static_cast<char*>(msg->payload);
	}

	if (topic == "weather/hello") {
	    try {
			m_helloCbk();
	    } catch(const std::bad_function_call& e) {
	        std::cerr << e.what() << "\n";
	        return;
	    }
	}
	if (topic.find(alldata) != std::string::npos) {
	    try {
			m_allDataCbk();
	    } catch(const std::bad_function_call& e) {
	        std::cerr << e.what() << "\n";
	        return;
	    }
	}
	if (topic.find(sensordata) != std::string::npos) {
	    try {
			m_sensorDataCbk(payload);
	    } catch(const std::bad_function_call& e) {
	        std::cerr << e.what() << "\n";
	        return;
	    }
	}

}

void MessageBroker::on_subcribe(int mid, int qos_count, const int *granted_qos)
{
	std::cout << __PRETTY_FUNCTION__ << ": Subscription succeeded" << std::endl;
}

void MessageBroker::on_error()
{
	std::cerr << __PRETTY_FUNCTION__ << ": Error occurred" << std::endl;
}

void MessageBroker::sendMessage(std::string &topic, std::string &payload)
{
	char buf[128];

	if (!m_connected) {
		std::cerr << __PRETTY_FUNCTION__ << ": Not connected, message is being discarded" << std::endl;
		return;
	}
	memset(buf, '\0', 128);

	for (int i = 0; i < payload.length(); i++) {
		buf[i] = payload[i];
	}
	publish(NULL, topic.c_str(), payload.length(), (uint8_t*)buf);
}

void MessageBroker::subscribeTopic(std::string topic)
{
	subscribe(NULL, topic.c_str());
}

