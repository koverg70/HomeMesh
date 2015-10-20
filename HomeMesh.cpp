#define UBRRH // if you need Serial
#include <Arduino.h>
#include <Print.h>
#include "printf.h"
#include "avr_sleep.h"

#include "RF24Network.h"
#include "RF24.h"
#include "nRF24L01.h"
#include "RF24Mesh.h"
#include "SPI.h"
#include "dht.h"
#include "Time/Time.h"

#include "node_id.h"

#define LED_PIN 		2	// the status LED pin
#if NODE_ID != 0
#define SCHEDULE_PIN 	14	// the relay pin
#if NODE_ID >= 10 && NODE_ID < 20
#define DHT22_PIN 		8 	// small sensor node 10..19 (RFTempSens kapcsolás)
#else
#define DHT22_PIN 		15 	// define if you want to read DHT sensor
#endif
#endif

/***** Configure the chosen CE,CS pins *****/
RF24 radio(9, 10);
RF24Network network(radio);
RF24Mesh mesh(radio,network);

#include "Components/itask.h"			// base interface for tasks
#include "Components/time_sync.h"		// all node use time syncronization

#if NODE_ID == 0
#include "Components/master.h"
#else
#include "Components/sensors.h"
#ifdef SCHEDULE_PIN
#include "Components/schedule.h"
#endif
#endif

uint8_t taskCount = 0;
ITask* tasks[10];

void addTask(ITask *task) {
	if (taskCount < sizeof(tasks))
	{
		tasks[taskCount++] = task;
		printf("Added task: %s\r\n", task->name());
	}
}

void setup() {
	sleep_setup();	// if you want to use sleep, or use the optiboot watchdog

	Serial.begin(115200);
	printf_begin();
	printf("HomeMesh 1.0\r\n");
	printf("(c) koverg70 %s %s\r\n", __DATE__, __TIME__);
	printf("NodeID: %d\r\n", NODE_ID);

	pinMode(LED_PIN, OUTPUT);

	mesh.setNodeID(NODE_ID);
	mesh.begin();

	// --- now we initialize the tasks the are regularily called with new messages and to process information ----

	addTask(new TimeSync(60000, 0));		// sync frequency and target node to require time from
#if NODE_ID == 0
	addTask(new Master());					// TODO: where to store received data
#else
	addTask(new Sensors(0));				// the target node to send sensor data to
#endif
#ifdef SCHEDULE_PIN
	addTask(new Schedule(SCHEDULE_PIN));	// sync frequency and target node to require time from
#endif

	// ------------------------------------------------------------------------------------------------------------

  	for (int i = 0; i < taskCount; ++i) {
  		tasks[i]->begin();
		printf("Task started: %s\r\n", tasks[i]->name());
  	}

}

void loop(void) {
	  // Call mesh.update to keep the network updated
	  mesh.update();
#if NODE_ID == 0
	  // In addition, keep the 'DHCP service' running on the master node so addresses will
	  // be assigned to the sensor nodes
	  mesh.DHCP();
#endif

	  // Check for incoming data from the sensors
	  while (network.available()) {
	    RF24NetworkHeader header;
		uint8_t payload[30];
	    //network.peek(header);
	    boolean ok = network.read(header, &payload, sizeof(payload));
	    if (ok)
	    {
	    	for (int i = 0; i < taskCount; ++i) {
	    		ok |= tasks[i]->receiveMessage(header, payload);
	    	}
		    if (!ok)
		    {
		    	printf("Received unknown message: %s\r\n", header.toString());
		    }
	    }
	    else
	    {
	    	printf("Error reading network.");
	    }
	  }

  	for (int i = 0; i < taskCount; ++i) {
  		tasks[i]->updateState();
  	}
}
