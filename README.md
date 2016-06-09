# ESP8266-MQTT-client
A wifi based MQTT client

This MQTT client will connect over Wifi to the MQTT broker and controls a digital output (LED, relay):
- toggle output and send status message on local button press
- receive messages from the MQTT broker to control output, change settings and query state
- periodically send status messages

Several nodes can operate within the same network; each node has a unique node ID.
On startup the node operates with default values, as set during compilation.
Hardware used is a ESP8266 WiFi module that connects directly to the MQTT broker.

Message structure is equal to the RFM69-based gateway/node sytem by the same author.
This means both type of gateway/nodes can be used in a single Openhab system.
For a decription of the message format look for the explanation pdf in the RFM69 gateway section.

The MQTT topic is /home/esp_gw/direction/nodeid/devid
	where direction is "sb" towards the node and "nb" towards the MQTT broker.

Defined devices are:
- 0	uptime:		read uptime in minutes
- 1	interval:	read/set transmission interval for push messages
- 2	RSSI:		read signal strength
- 3	version:	read software version
- 3	version:	read input voltage
- 5	ACK:		read/set acknowledge message after a 'set' request
- 6	toggle:		read/set select toggle / timer function
- 7	timer:		read/set timer interval in seconds
- 10	IP:		read IP address of node
- 16	actuator:	read/set LED or relay output
- 40	button:		tx only: message sent when button pressed
- 92	error:		tx only: device not supported
- 91	error:		tx only: syntax error
- 99	wakeup:		tx only: first message sent on node startup

Hardware connections:

- pin 0 is connected to a button that switches to GND, pullup to VCC
- pin 2 is connected to LED and current limiting resistor to GND

Version 1.2 by computourist@gmail.com June 2016

