/*
 * MessageBroker.h
 *
 *  Created on: Jan 16, 2019
 *      Author: Peter Buelow
 */

#ifndef MESSAGEBROKER_H_
#define MESSAGEBROKER_H_

#include <iostream>
#include <string>
#include <functional>
#include <cstring>
#include <mutex>
#include <mosquittopp.h>

#define MAX_PAYLOAD 50

class MessageBroker : public mosqpp::mosquittopp {
public:
	MessageBroker(const char*, const char*, int);
	virtual ~MessageBroker();

	void sendMessage(std::string&, std::string&);
	int doLoop() { loop(); }
	void setHelloCallback(std::function<void()> cbk) { m_helloCbk = cbk; }
	void setAllDataCallback(std::function<void()> cbk) { m_allDataCbk = cbk; }
	void setSensorDataCallback(std::function<void(std::string)> cbk) { m_sensorDataCbk = cbk; }
	void subscribeTopic(std::string);
	void on_connect(int);
	void on_message(const struct mosquitto_message*);
	void on_subcribe(int, int, const int*);
	void on_disconnect(int);
	void on_error();
	void setMutex(std::mutex *m);

private:

	std::string m_name;
	std::string m_host;
	int m_port;
	bool m_connected;
	std::mutex *m_mutex;
	std::function<void()> m_helloCbk;
	std::function<void()> m_allDataCbk;
	std::function<void(std::string)> m_sensorDataCbk;
};

#endif /* MESSAGEBROKER_H_ */
