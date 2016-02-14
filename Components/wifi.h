/*
 * wifi.h
 *
 *  Created on: 2015.11.15.
 *      Author: koverg
 */

#ifndef WIFI_H_
#define WIFI_H_

#include "ESPServer.h"

#define RSTR(x)	x

/*
static const char respOK[] PROGMEM = ("HTTP/1.0 200 OK\r\n"
				"Content-Type: text/json\r\n"
				"Pragma: no-cache\r\n"
				"Access-Control-Allow-Origin: *\r\n"	// needed to allow request from other sites
				"\r\n");
*/

class Wifi : public ITask
{
private:
	ESPServer *esp8266;
	SensorStore *sensors;
	unsigned long startMillis;
	time_t startTime;
public:
	Wifi(SensorStore *sensors_) {
		sensors = sensors_;
		startMillis = millis();
		esp8266 = NULL;
		startTime = 0;
	}

	const char *name() { return "Wifi_ESP8266"; };

    virtual boolean begin(void) {
    }

	void updateState() {
		// TODO: jobb lenne egy callback arra, hogy be lett állítva az idõ
		if (year() > 2013 && startTime == 0) {
			startTime = now();
		}

		if (esp8266 == NULL) {
			if (millis() - startMillis > 2000) {
				printf_P(PSTR("Wifi init started."));
				esp8266 = new ESPServer(2, 3, ESP8266_BAUD);
				delay(50);
				esp8266->start(80);
			}
		} else {
			uint16_t len;
			char command[64];
			int id = esp8266->receive(100, len);
			if (id >= 0) {
				long start = millis();
				do {
		#ifdef DEBUG_SERIAL
					Serial.println(len);
		#endif
					char buffer[101];
					int read = esp8266->readBuffer(id, buffer, 100, '\n', DEFAULT_TIMEOUT);
					if (strncmp(buffer, "GET ", 4) == 0) {
						char *p = buffer + 4;
						while (*p != 0 && *p != ' ') ++p;
						*p = 0;
						Serial.print("Request: ");
						Serial.println(buffer + 4);
						strncpy(command, buffer + 4, sizeof(command)-1);
					}
					len -= read;
				} while (len > 0 && millis() - start < DEFAULT_TIMEOUT);
				if (command[0] == '/' && command[1] == 'V' && command[2] == 0) {
					print_webpage(id);
				} else if (command[0] == '/' && command[1] == 'T') {
					int year, month, day, hour, min, sec;
					if (6 == sscanf(command+2, "%d.%d.%d.%d:%d:%d", &year, &month, &day, &hour, &min, &sec)) {
						setTime(hour,min,sec,day,month,year);
						esp8266->sendResponse(id, "OK", false);
					} else {
						esp8266->sendResponse(id, "Bad format.", false);
					}
				} else {
					esp8266->sendResponse(id, "HTTP/1.1 404 Not Found", false);
				}

				esp8266->endResponse(id);
			}
		}
	}

	boolean receiveMessage(RF24NetworkHeader header, uint8_t *payload) {
		return false;
	}

	void timeDateToText(time_t time, char *buffer)
	{
		// YEAR
		int val = year(time);
		buffer[0] = '0' + (val / 1000);
		val -= (val / 1000) * 1000;
		buffer[1] = '0' + (val / 100);
		val -= (val / 100) * 100;
		buffer[2] = '0' + (val / 10);
		val -= (val / 10) * 10;
		buffer[3] = '0' + val;
		buffer[4] = '.';

		// MONTH
		val = month(time);
		buffer[5] = '0' + (val / 10);
		val -= (val / 10) * 10;
		buffer[6] = '0' + val;
		buffer[7] = '.';

		// DAY
		val = day(time);
		buffer[8] = '0' + (val / 10);
		val -= (val / 10) * 10;
		buffer[9] = '0' + val;
		buffer[10] = '.';
		buffer[11] = ' ';

		// HOUR
		val = hour(time);
		buffer[12] = '0' + (val / 10);
		val -= (val / 10) * 10;
		buffer[13] = '0' + val;
		buffer[14] = ':';

		// MIN
		val = minute(time);
		buffer[15] = '0' + (val / 10);
		val -= (val / 10) * 10;
		buffer[16] = '0' + val;
		buffer[17] = ':';

		// SEC
		val = second(time);
		buffer[18] = '0' + (val / 10);
		val -= (val / 10) * 10;
		buffer[19] = '0' + val;

		buffer[20] = 0;
	}

	void print_webpage(uint8_t id)
	{
		char timeBuff[30];

		time_t nnn = now();
		esp8266->appendResponse(id, RSTR("{\"time\": \""));
		timeDateToText(nnn, timeBuff);
		esp8266->appendResponse(id, timeBuff);
		esp8266->appendResponse(id, RSTR("\", \"start\": \""));
		ltoa(startTime, timeBuff, 10);
		esp8266->appendResponse(id, timeBuff);
		esp8266->appendResponse(id, ("\""));

		if (sensors != NULL)
		{
			for (int j = 0; j < sensors->getNodeCount(); ++j)
			{
				NodeData *node = sensors->getNodes()[j];
				for (int i = 0; i < sensors->getSensorCount(); ++i)
				{
					/*
					uint8_t		nodeId;			// the node that collected the data
					char		sensorType;		// A,B,C,D,E,F,G,H,I,J - temperature, X,Y,Z - humidity, 0,1,2,3,4,5,6,7,8,9 - other values
					time_t 		lastUpdate; 	// the date and time of the last sensor message
					uint16_t	value;			// the sensor value
					uint16_t	extra;			// extra data (if value cannot be stored on 16 bit)
					*/


					SensorData * sensor = (sensors->getSensors())[i];
					itoa(i, timeBuff, 10);
					esp8266->appendResponse(id, ", \"sensors");
					esp8266->appendResponse(id, timeBuff);
					esp8266->appendResponse(id, RSTR("\": {"));

					esp8266->appendResponse(id, ("\"nodeId\": \""));
					itoa(sensor->nodeId, timeBuff, 10);
					esp8266->appendResponse(id, timeBuff);

					esp8266->appendResponse(id, RSTR("\", \"lastUpdate\": \""));
					itoa(nnn - sensor->lastUpdate, timeBuff, 10);
					esp8266->appendResponse(id, timeBuff);

					esp8266->appendResponse(id, RSTR("\", \"sensorType\": \""));
					timeBuff[0] = sensor->sensorType;
					timeBuff[1] = 0;
					esp8266->appendResponse(id, timeBuff);

					esp8266->appendResponse(id, RSTR("\", \"value\": \""));
					itoa(sensor->value, timeBuff, 10);
					esp8266->appendResponse(id, timeBuff);

					esp8266->appendResponse(id, RSTR("\", \"extra\": \""));
					itoa(sensor->extra, timeBuff, 10);
					esp8266->appendResponse(id, timeBuff);

					esp8266->appendResponse(id, RSTR("\"}"));
				}
			}
		}
		esp8266->appendResponse(id, RSTR("}"));

//		plen = es.ES_fill_tcp_data_p(buf, plen, (", settings: \""));
//		for (int i = 0; i < SETTINGS_SIZE; ++i)
//		{
//			byte b = EEPROM.read(i);
//			//byte b = settings[i];
//			if (b != '0' && b != '1')
//			{
//				b = '0';
//			}
//			timeBuff[0] = b;
//			timeBuff[1] = 0;
//			plen = es.ES_fill_tcp_data(buf,plen, timeBuff);
//		}
//
//		plen = es.ES_fill_tcp_data_p(buf, plen, ("\"}\0"));
	}

};

#endif /* WIFI_H_ */
