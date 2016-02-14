/*
 * ESPServer.h
 *
 * A simple class that communicates with the ESP8266 AT interface to setup and
 * run a simple web server.
 *
 * I created this class because every other library on the Interent used WString from
 * Arduino which increased the binary size of the code.
 *
 *  Created on: 2015.11.25.
 *      Author: koverg
 */
#include <Arduino.h>
//#include "SoftwareSerial/SoftwareSerial.h"

#ifndef ESPSERVER_H_
#define ESPSERVER_H_

#define DEFAULT_TIMEOUT 250
#define ESP8266_BAUD 115200
//#define DEBUG_SERIAL

#define ESPSerial Serial1

class ESPServer
{
private:
	char espbuff[256];

	void sendCommand(const char* buffer, ...);
	boolean waitFor(const char* buffer, long timeout);
	int readInt(long timeout, char end);

public:
	ESPServer(uint8_t rx, uint8_t tx, long baud);
	void start(int port);
	void stop();
	int receive(long timeout, uint16_t &length);
	int readBuffer(int id, char *buffer, int length, char terminator, long timeout);
	boolean sendResponse(int id, char* buffer, boolean close);
	boolean appendResponse(int id, char* buffer);
	boolean endResponse(int id);
	void status();
};

#endif /* ESPSERVER_H_ */
