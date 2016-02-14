/*
 * ESPServer.cpp
 *
 *  Created on: 2015.11.25.
 *      Author: koverg
 */
#include "ESPServer.h"

void ESPServer::sendCommand(const char* buffer, ...)
{
	char dest[256];
	if (strlen(buffer) > 200)
	{
		// TODO ERROR
	}
	va_list argptr;
	va_start(argptr, buffer);
	vsprintf(dest, buffer, argptr);
	va_end(argptr);
//	    Serial.println(dest);
	ESPSerial.write(dest);
}


boolean ESPServer::waitFor(const char* buffer, long timeout) {
	long start = millis();
	int pos = 0;
	boolean found = false;
	while (millis() - start < timeout) {
		while (ESPSerial.available() > 0) {
			char c = ESPSerial.read();
#ifdef DEBUG_SERIAL
			Serial.print(c);
#endif
#ifdef WAIT_HEX_DEBUG
			char bbb[10];
			itoa(c, bbb, 16);
			Serial.print(" (");
			Serial.print(bbb);
			Serial.print(")\r\n");
#endif
			if (c == buffer[pos]) {
				++pos;
#ifdef WAIT_HEX_DEBUG
				Serial.println('+');
#endif
			}
			else
			{
#ifdef WAIT_HEX_DEBUG
				Serial.println('-');
#endif
				pos = 0;
				if (c == buffer[pos]) {
					++pos;
#ifdef WAIT_HEX_DEBUG
					Serial.println('x');
#endif
				}
			}
			if (buffer[pos] == 0) {
#ifdef WAIT_HEX_DEBUG
				Serial.println('!');
#endif
				found = true;
				break;
			}
		}
		if (found) {
			break;
		}
		delay(10);
	}

	return found;
}


int ESPServer::readInt(long timeout, char end)
{
	int ret = 0;
	long start = millis();
	while (millis() - start < timeout)
	{
		while (ESPSerial.available() > 0)
		{
			char c = ESPSerial.read();
#ifdef DEBUG_SERIAL
			Serial.print(c);
#endif
			if (c == end)
			{
				return ret;
			}
			if (isDigit(c))
			{
				ret = 10*ret + (c-'0');
			}
		}
		delay(10);
	}
	return -1;
}

ESPServer::ESPServer(uint8_t rx, uint8_t tx, long baud)
{
//	this->esp8266 = new SoftwareSerial(rx, tx);
//		this->esp8266->begin(9600);
//		sendData("AT+UART_CUR=%d", baud);
//		if (waitFor("\r\nOK\r\n", DEFAULT_TIMEOUT)) {
//			Serial.println("Baud set OK.");
//		} else {
//			Serial.println("Baud set fail.");
//		}
//		this->esp8266->end();
//		delay(500);
	ESPSerial.begin(baud);
	espbuff[0] = 0;
}

void ESPServer::start(int port) {
	//delay(2000);
	sendCommand("AT+CWMODE_CUR=%d\r\n",3);
	delay(10);
	if (waitFor("\r\nOK\r\n", DEFAULT_TIMEOUT)) {
		Serial.println("CWMODE=3 success.");
	} else {
		Serial.println("CWMODE=3 fail.");
	}
	sendCommand("AT+CIPMUX=%d\r\n",1);
	delay(10);
	if (waitFor("\r\nOK\r\n", DEFAULT_TIMEOUT)) {
		Serial.println("MUX=1 success.");
	} else {
		Serial.println("MUX=1 fail.");
	}
	sendCommand("AT+CIPSERVER=1,%d\r\n", port);
	delay(10);
	if (waitFor("\r\nOK\r\n", DEFAULT_TIMEOUT)) {
		Serial.println("Web server started.");
	} else {
		Serial.println("Web server not started.");
	}

}

void ESPServer::stop() {
	sendCommand("AT+CIPSERVER=0\r\n");
	waitFor("\r\nOK\r\n", DEFAULT_TIMEOUT);
}

int ESPServer::receive(long timeout, uint16_t &length) {
	if (waitFor("CONNECT\r\n", timeout))
	{
		if (waitFor("+IPD,", timeout))
		{
			int id = readInt(timeout, ',');
			length = readInt(timeout, ':');

			return id;
		}
	}
	return -1;
}

int ESPServer::readBuffer(int id, char *buffer, int length, char terminator, long timeout)
{
	int pos = 0;
	long start = millis();
	while (pos < length && millis() - start < timeout)
	{
		while (ESPSerial.available() > 0 && pos < length)
		{
			char c = ESPSerial.read();
#ifdef DEBUG_SERIAL
			Serial.print(c);
#endif
			buffer[pos++] = c;
			if (c == terminator)
			{
				buffer[pos] = 0;
				return pos;
			}
		}
		delay(10);
	}
	buffer[pos] = 0;
	return pos;
}

boolean ESPServer::sendResponse(int id, char* buffer, boolean close) {
	int len = strlen(buffer);
	sendCommand("AT+CIPSEND=%d,%d\r\n", id, len);
	if (waitFor("\r\nOK\r\n>", DEFAULT_TIMEOUT)) {
		sendCommand(buffer);
#ifdef DEBUG_SERIAL
		Serial.print("#send:#");
		Serial.println(buffer);
#endif
		if (waitFor("\r\nSEND OK\r\n", DEFAULT_TIMEOUT)) {
			if (close) {
				sendCommand("AT+CIPCLOSE=%d\r\n", id);
				return waitFor("\r\nOK\r\n", DEFAULT_TIMEOUT);
			}
			return true;
		}
	}
	return false;
}

boolean ESPServer::appendResponse(int id, char *buffer) {
	int count = strlen(buffer);
	int send;
	while (count > 0) {
		send = count;
		if (send + strlen(espbuff) + 1 > sizeof(espbuff)) {
			send = sizeof(espbuff) - strlen(espbuff) - 1;
			strncat(espbuff, buffer, send);
			espbuff[sizeof(espbuff)-1] = 0;
			if (!sendResponse(id, espbuff, false)) {
				return false;
			}
			espbuff[0] = 0;
		} else {
			strcat(espbuff, buffer);
		}

		count -= send;
		buffer += send;
	}
	return true;
}

boolean ESPServer::endResponse(int id) {
	if (strlen(espbuff) > 0) {
		if (!sendResponse(id, espbuff, false)) {
			return false;
		}
		espbuff[0] = 0;
	}
	sendCommand("AT+CIPCLOSE=%d\r\n", id);
	return waitFor("\r\nOK\r\n", DEFAULT_TIMEOUT);
}

void ESPServer::status() {
	waitFor("", DEFAULT_TIMEOUT);
	sendCommand("AT\r\n");
	waitFor("\r\nOK\r\n", DEFAULT_TIMEOUT);
}
