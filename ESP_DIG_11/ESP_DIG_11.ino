//	ESP_DIG_V1.1
//
//	This MQTT client will connect over Wifi to the MQTT broker and controls a digital output (LED, relay):
//	- toggle output and send status message on local button press
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
//	5	ACK:		read/set acknowledge message after a 'set' request
//	6	toggle:		read/set select toggle / timer function
//	7	timer:		read/set timer interval in seconds
//	10  IP:      read ip address of the ESP
//	16	actuator:	read/set LED or relay output
//	40	button		tx only: button pressed
//	92	error:		tx only: device not supported
//	91	error:		tx only: syntax error
//	99	wakeup:		tx only: first message sent on node startup
//
//	Hardware connections:
//	-	pin 0 is connected to a button that switches to GND, pullup to VCC
//	-	pin 2 is connected to LED and current limiting resistor to GND
//
//	version 1.0 by computourist@gmail.com May 2016
//	version 1.1 May 2016
//		- some minor bug fixes and code improvements
//
	#include <ESP8266WiFi.h>
	#include <PubSubClient.h>
	#include <IPAddress.h>
	#define VERSION "WDIG V1.1"						// this value can be queried as device 3
	#define wifi_ssid "xxxx"						// wifi station name
	#define wifi_password "xxxxxxxx"				// wifi password
	#define mqtt_server "192.168.xx.xx"				// mqtt server IP
	#define nodeId 80								// node ID

	//	sensor setting

	#define ACT1 2									// Actuator pin (LED or relay to ground)
	#define BTN 0									// Button pin (also used for Flash setting)
	#define SERIAL_BAUD 115200
	#define HOLDOFF 1000							// blocking period between button messages

	//	STARTUP DEFAULTS

	long 	TXinterval = 20;						// periodic transmission interval in seconds
	long	TIMinterval = 20;						// timer interval in seconds
	bool	ackButton = false;						// flag for message on button press
	bool	toggleOnButton = true;					// toggle output on button press

	//	VARIABLES

	int		DID;									// Device ID
	int		error;									// Syntax error code
	long	lastPeriod = -1;						// timestamp last transmission
	long 	lastBtnPress = -1;						// timestamp last buttonpress
	long	lastMinute = -1;						// timestamp last minute
	long	upTime = 0;								// uptime in minutes
	int		ACT1State;								// status ACT1 output
	String  IP;
	bool	wakeUp = true;							// wakeup indicator
	bool	setAck = false; 						// acknowledge receipt of actions
	bool	curState = true;						// current button state 
	bool	lastState = true;						// last button state
	bool	timerOnButton = false;					// timer output on button press
	bool	msgBlock = false;						// flag to hold button message
	bool	readAction;								// indicates read / set a value
	bool	send0, send1, send3, send5, send6, send7, send10;
	bool	send16, send40, send99;					// message triggers
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
		if (DID==3) {								// version
			if (readAction) {
				send3 = true;
				error = 0;
			} else error = 3;						// invalid payload; do not process
		}
		if (DID==5) {								// ACK
			if (readAction) {
				send5 = true;
				error = 0;
			} else if (strPayload == "ON") {
					setAck = true;
					if (setAck) send5 = true;
					error = 0;
			}	else if (strPayload == "OFF") {
					setAck = false;
					if (setAck) send5 = true;
					error = 0;
			}	else error = 3;
		}
		
		if (DID==6) {								// toggle / timer mode selection
			if (readAction) {
				send6 = true;
				error = 0;
			} else if (strPayload == "ON") {		// select toggle mode
					toggleOnButton = true;
					if (setAck) send6 = true;
					error = 0;
			}	else if (strPayload == "OFF") {		// select timer mode
					toggleOnButton = false;
					if (setAck) send6 = true;
					error = 0;
			}	else error = 3;
		}
		
		if (DID==7) {								// Timer interval
			error = 0;
			if (readAction) {
				send7 = true;
			} else {								// set timer interval
				TIMinterval = strPayload.toInt();
				if (TIMinterval <5 && TIMinterval !=0) TIMinterval = 5;	// minimum interval is 5 seconds
			}
		}

		if (DID ==10) {                // IP address 
			if (readAction) {
				send0 = true;
				error = 0;
			} else error = 3;           // invalid payload; do not process
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

	if (send3) {									// send software version
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev03", nodeId);
		for (i=0; i<sizeof(VERSION); i++) {
			buff_msg[i] = VERSION[i];}
		buff_msg[i] = '\0';
		send3 = false;
		pubMQTT(buff_topic, buff_msg);
		}

	if (send5) {									// send ACK state
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev05", nodeId);
		if (!setAck) sprintf(buff_msg, "OFF");
		else sprintf(buff_msg, "ON");
		pubMQTT(buff_topic, buff_msg);
		send5 = false;
	}
	
	if (send6) {									// send toggleOnButton state
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev06", nodeId);
		if (!toggleOnButton) sprintf(buff_msg, "OFF");
		else sprintf(buff_msg, "ON");
		pubMQTT(buff_topic, buff_msg);
		send6 = false;
	}
	
	if (send7) {									// send timer value
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev07", nodeId);
		sprintf(buff_msg, "%d", TIMinterval);
		pubMQTT(buff_topic, buff_msg);
		send7 = false;
	}

	if (send10) {                 // send IP address
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev10", nodeId);
		IP = WiFi.localIP().toString();
		for (i=0; i<sizeof(IP); i++) {
			buff_msg[i] = IP[i];}
		buff_msg[i] = '\0';
		send10 = false;
		pubMQTT(buff_topic, buff_msg);
	}
		
	if (send16) {									// send actuator state
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev16", nodeId);
		if (ACT1State ==0) sprintf(buff_msg, "OFF");
		if (ACT1State ==1) sprintf(buff_msg, "ON");
		pubMQTT(buff_topic, buff_msg);
		send16 = false;
	}
	
	if (send40) {									// send button pressed message
		sprintf(buff_topic, "home/esp_gw/nb/node%02d/dev40", nodeId);
		if (ACT1State ==0) sprintf(buff_msg, "OFF");
		if (ACT1State ==1) sprintf(buff_msg, "ON");
		pubMQTT(buff_topic, buff_msg);
		send40 = false;
	}
	
}

	//	SETUP

//===============================================================================================
	void setup() {									// set up serial, output and wifi connection
		pinMode(ACT1,OUTPUT);						// configure output
		ACT1State = 0;
	send0 = false;
	send1 = false;
	send3 = false;
	send5 = false;
	send7 = false;
	send10 = false;
	send16 = false;
	send40 = false;
		digitalWrite(ACT1,ACT1State);
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
		Serial.println(WiFi.localIP());
		}

	//	LOOP

//===============================================================================================
	void loop() {									// Main program loop
	if (!client.connected()) {
		reconnect();
		}
	client.loop();

		// DETECT INPUT CHANGE

	curState = digitalRead(BTN);								// Read button
	msgBlock = ((millis() - lastBtnPress) < HOLDOFF);			// hold-off time for additional button messages
	if (!msgBlock &&  (curState != lastState)) {				// input changed ?
		delay(5);
//		Serial.println("Button state changed.. ");
		lastBtnPress = millis();								// take timestamp
		if (setAck) send40 = true;								// send button message
		if (curState == LOW) {
		if (toggleOnButton) {									// button in toggle state ?
			ACT1State = !ACT1State; 							// toggle output
			digitalWrite(ACT1, ACT1State);
			send16 = true;										// send message on status change
		} else
		if (TIMinterval > 0 && !timerOnButton) {				// button in timer state ?
			timerOnButton = true;								// start timer interval
			ACT1State = HIGH;									// switch on ACT1
			digitalWrite(ACT1, ACT1State);
			send16 = true;
		}}
		lastState = curState;
		}

		// TIMER CHECK

	if (TIMinterval > 0 && timerOnButton) {						// =0 means no timer
		if ( millis() - lastBtnPress > TIMinterval*1000) {		// timer expired ?
			timerOnButton = false;								// then end timer interval 
			ACT1State = LOW;									// and switch off Actuator
			digitalWrite(ACT1, ACT1State);
			send16 = true;
			}
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
 
//			send0 = true;
			send3 = true;										// send version
			send10 = true;
			send16 = true;										// output state
			}
		}

	sendMsg();													// send any mqtt messages

}		// end loop
    

