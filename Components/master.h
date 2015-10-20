/*
 * master.h
 *
 *  Created on: 2015.10.17.
 *      Author: koverg
 */

#ifndef MASTER_H_
#define MASTER_H_

class Master : public ITask
{
private:
	uint32_t addressTimer;
public:
	Master() {
		addressTimer = 0;
	}

	boolean receiveMessage(RF24NetworkHeader header, uint8_t *payload) {
		if (header.type == 'V') {
		  printf("Received sensor from device:%#o sensor:%c value:%d\r\n", header.from_node, (char)payload[0], *((uint16_t *)(payload + 1)));
		  return true;
		}
		return false;
	}

	const char *name() { return "Master"; };

	void updateState() {
		  if(millis() - addressTimer > 15000){
			addressTimer = millis();
			Serial.println(" ");
			Serial.println("********Assigned Addresses********");
			 for(int i=0; i<mesh.addrListTop; i++){
			   Serial.print("NodeID: ");
			   Serial.print(mesh.addrList[i].nodeID);
			   Serial.print(" RF24Network Address: 0");
			   Serial.println(mesh.addrList[i].address, 8);
			 }
			Serial.println("**********************************");

			if (year(now()) <= 2013) {
				uint8_t buffer[5];
				buffer[0] = 'G';
				mesh.write(buffer, 'T', sizeof(buffer), 1);
				printf("Time request sent.\r\n");
			}
			else {
				// --- debug set schedule ---
				uint8_t buffer[13];
				buffer[0] = 'S';
				for (int i = 0; i < 10; ++i) { buffer[1 + i] = 0x00; }
				for (int i = 10; i < 12; ++i) { buffer[1 + i] = 0xff; }
				mesh.write(buffer, 'S', sizeof(buffer), 1);
				printf("Schedule message sent.\r\n");
				// --- debug set schedule ---
			}

		  }
	}
};

#endif /* MASTER_H_ */
