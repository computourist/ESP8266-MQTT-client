//	ESP_BAS_V1.0
//
//	Very Basic MQTT client. 
//
//	This MQTT client will connect over Wifi to the MQTT broker and send switch status and controls digital output (LED, relay):
//	- end status message on local switch position
//	- receive messages from the MQTT broker to control output, change settings and query state
//	- periodically send status messages
//
//	Several nodes can operate within the same network; each node has a unique node ID.
//	On startup the node operates with default values, as set during compilation.
//	Hardware used is a ESP8266 WiFi module that connects directly to the MQTT broker.
//
//	Message structure is equal to the RFM69-based gateway/node sytem by the same author.
//	This means both type of gateway/nodes can be used in a single Openhab system.
//
//	The MQTT topic is /home/esp_gw/direction/nodeid/devid
//	where direction is "sb" towards the node and "nb" towards the MQTT broker.
//
//	Defined devices are:
//	0	uptime:		read uptime in minutes
//	1	interval:	read/set transmission interval for push messages
//	3	version:	read software version
//	2	RSSI:		read radio signal strength
//	10	IP:			Read IP address
//	16	actuator:	read/set LED or relay output
//	40	input:		tx only: switch position
//	92	error:		tx only: device not supported
//	91	error:		tx only: syntax error
//	99	wakeup:		tx only: first message sent on node startup
//
//	Hardware connections:
//	-	pin 0 is connected to a switche to GND, pullup to VCC
//	-	pin 2 is connected to LED and current limiting resistor to GND
//
//	version 1.0 by computourist@gmail.com June 2016
//
	#include <ESP8266WiFi.h>
	#include <PubSubClient.h>
	#define VERSION "WBAS V1.0"						// this value can be queried as device 3
	#define wifi_ssid "xxxxx"						// wifi station name
	#define wifi_password "xxxxxxxx"				// wifi password
	#define mqtt_server "192.168.xxx.xxx"			// mqtt server IP
	#define nodeId 80								// node ID

	//	sensor setting

	#define ACT1 2									// Actuator pin (LED or relay to ground)
	#define INP1 0									// Input pin (0 is also used for Flash setting)
	#define SERIAL_BAUD 115200
	#define HOLDOFF 1000							// blocking period between button messages

	//	STARTUP DEFAULTS

	long	TXinterval = 120;						// periodic transmission interval in seconds


	//	VARIABLES

	int		DID;									// Device ID
	int		error;									// Syntax error code
	long	lastPeriod = -1;						// timestamp last transmission
	long 	lastINPChange = -1;						// timestamp last input change
	long	lastMinute = -1;						// timestamp last minute
	long	upTime = 0;								// uptime in minutes
	int		ACT1State;								// status ACT1 output
	int		Inp1State;								// status input1
	int		signalStrength;							// radio signal strength
	bool	wakeUp = true;							// wakeup indicator
	bool	setAck = true; 							// acknowledge receipt of output commands
	bool	curInp1 = true;							// current input 1 state 
	bool	lastInp1 = true;						// last input 1 state
	bool	msgBlock = false;						// flag to hold input message
	bool	readAction;								// indicates read / set a value
	bool	send0, send1, send2, send3;
	bool	send10, send16, send40, send99;			// message triggers
	String	IP;										// IPaddress of ESP
	char	buff_topic[30];							// mqtt topic
	char	buff_msg[32];							// mqtt message

void mqttSubs(char* topic, byte* payload, unsigned int length);

	WiFiClient espClient;
	PubSubClient client(mqtt_server, 1883, mqttSubs, espClient); // instantiate MQTT client
	
	//	FUNCTIONS

//===============================================================================================

void pubMQTT(String topic, String topic_val){ // publish MQTT message to broker
	Serial.print("topic " + topic + " value:");
	Serial.println(String(topic_val).c_str());
	client.publish(topic.c_str(), String(topic_val).c_str(), true);
	}

void mqttSubs(char* topic, byte* payload, unsigned int length) {	// receive and handle MQTT messages
	int i;
	error = 4; 										// assume invalid device until proven otherwise
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
		}
	Serial.println();
	if (strlen(topic) == 27) {						// correct topic length ?
		DID = (topic[25]-'0')*10 + topic[26]-'0';	// extract device ID from MQTT topic
		payload[length] = '\0';						// terminate string with '0'
		String strPayload = String((char*)payload);	// convert to string
	readAction = (strPayload == "READ");			// 'READ' or 'SET' value
	if (length == 0) {error = 2;}					// no payload sent
	else {
		if (DID ==0) {								// uptime 
			if (readAction) {
				send0 = true;
				error = 0;
			} else error = 3;						// invalid payload; do not process
		}
		if (DID==1) {								// transmission interval
			error = 0;
			if (readAction) {
				send1 = true;
			} else {								// set transmission interval
				TXinterval = strPayload.toInt();
				if (TXinterval <10 && TXinterval !=0) TXinterval = 10;	// minimum interval is 10 seconds
			}
		}
		if (DID ==2) {								// RSSI
			if (readAction) {
				send2 = true;
				error = 0;
			} else error = 3;						// invalid payload; do not process
		}
		if (DID==3) {								// version
			if (readAction) {
				send3 = true;
				error = 0;
			} else error = 3;						// invalid payload; do not process
		}
		
		if (DID ==10) {								// IP address 
			if (readAction) {
				send10 = true;
				error = 0;
			} else error = 3;						// invalid payload; do not process
		}
		if (DID==16) {								// state of actuator
			if (readAction) {
				send16 = true;
				error = 0;
			} else if (strPayload == "ON") {
					ACT1State = 1;
					digitalWrite(ACT1,ACT1State);
					if (setAck) send16 = true;
					error = 0;
			}	else if (strPayload == "OFF") {
					ACT1State = 0;
					digitalWrite(ACT1,ACT1State);
					if (setAck) send16 = true;
					error = 0;
			}	else error = 3;
		}
				if (DID==40) {								// state of actuator
			if (readAction) {
				send40 = true;
				error = 0;
			}	else error = 3;								// no write action allowed
		}
		
	}
		} else error =1;
		if (error !=0) {							// send error message
				sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev91", nodeId);
				sprintf(buff_msg, "syntax error %d", error);
				pubMQTT(buff_topic, buff_msg);
			}
		}

	void reconnect() {								// reconnect to mqtt broker
		sprintf(buff_topic, "home/esp_gw/sb/node%02d/#", nodeId);
		while (!client.connected()) {
			Serial.print("Connect to MQTT broker...");
			if (client.connect("ESP_GW")) { 
				client.subscribe(buff_topic);
				Serial.println("connected");
			} else {
				Serial.println("Failed, try again in 5 seconds");
				delay(5000);
				}
			}
		}

	void sendMsg() {								// send any outstanding messages
	int i;
	if (wakeUp) {									// send wakeup message
		wakeUp = false;
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev99", nodeId);
		sprintf(buff_msg, "NODE %d WAKEUP", nodeId);
		send99 = false;
		pubMQTT(buff_topic, buff_msg);
		}
	if (send0) {									// send uptime
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev00", nodeId);
		sprintf(buff_msg, "%d", upTime);
		send0 = false;
		pubMQTT(buff_topic, buff_msg);
		}

	if (send1) {									// send transmission interval
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev01", nodeId);
		sprintf(buff_msg, "%d", TXinterval);
		send1 = false;
		pubMQTT(buff_topic, buff_msg);
		}

	if (send2) {									// send signal strength
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev02", nodeId);
		signalStrength = WiFi.RSSI();
		sprintf(buff_msg, "%d", signalStrength);
		send2 = false;
		pubMQTT(buff_topic, buff_msg);
		}

	if (send3) {									// send software version
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev03", nodeId);
		for (i=0; i<sizeof(VERSION); i++) {
			buff_msg[i] = VERSION[i];}
		buff_msg[i] = '\0';
		send3 = false;
		pubMQTT(buff_topic, buff_msg);
		}
	
	if (send10) {								// send IP address
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev10", nodeId);
		for (i=0; i<16; i++) {
			buff_msg[i] = IP[i];}
		buff_msg[i] = '\0';
		pubMQTT(buff_topic, buff_msg);
		send10 = false;
	}
	if (send16) {									// send actuator state
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev16", nodeId);
		if (ACT1State ==0) sprintf(buff_msg, "OFF");
		if (ACT1State ==1) sprintf(buff_msg, "ON");
		pubMQTT(buff_topic, buff_msg);
		send16 = false;
	}
	
	if (send40) {									// send Input1 value
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev40", nodeId);
		if (curInp1 ==0) sprintf(buff_msg, "OFF");
		if (curInp1 ==1) sprintf(buff_msg, "ON");
		pubMQTT(buff_topic, buff_msg);
		send40 = false;
	}
	
}

	//	SETUP

//===============================================================================================
	void setup() {									// set up serial, output and wifi connection
	pinMode(ACT1,OUTPUT);						// configure output
	ACT1State = 0;
	digitalWrite(ACT1,ACT1State);
	send0 = false;
	send1 = false;
	send3 = false;
	send10 = true;									// send IP on startup
	send16 = false;
	send40 = false;
		Serial.begin(SERIAL_BAUD);
		Serial.println();							// connect to WIFI
		Serial.print("Connecting to ");
		Serial.println(wifi_ssid);
		WiFi.begin(wifi_ssid, wifi_password);
		while (WiFi.status() != WL_CONNECTED) {
			delay(500);
				Serial.print(".");
			}
		Serial.println("");
		Serial.println("WiFi connected");
		Serial.println("IP address: ");
		IP = WiFi.localIP().toString();
		Serial.println(IP);
		}

	//	LOOP

//===============================================================================================
	void loop() {									// Main program loop
	if (!client.connected()) {
		reconnect();
		}
	client.loop();
		// READ INPUT STATE

		curInp1 = digitalRead(INP1);							// Read input1
		msgBlock = ((millis() - lastINPChange) < HOLDOFF);		// hold-off time for additional button messages
	if (!msgBlock &&  (curInp1 != lastInp1)) {					// input changed ?
		delay(5);
		Serial.println("Input1 state changed.. ");
		lastINPChange = millis();								// take timestamp
		send40 = true;											// send button message
		lastInp1 = curInp1;
		}

		// INCREASE UPTIME 

	if (lastMinute != (millis()/60000)) {						// another minute passed ?
		lastMinute = millis()/60000;
		upTime++;
		}

		// PERIODIC TRANSMISSION


	if (TXinterval > 0){
		int currPeriod = millis()/(TXinterval*1000);
		if (currPeriod != lastPeriod) {							// interval elapsed ?
			lastPeriod = currPeriod;

			// list of sensordata to be sent periodically..
			// remove comment to include parameter in transmission
 
//			send10 = true;										// send IP address
			send2 = true;										// send RSSI
//			send3 = true;										// send version
			send16 = true;										// output state
			send40 = true;
			}
		}

	sendMsg();													// send any mqtt messages

}		// end loop
    
