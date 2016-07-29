# ESP8266-MQTT-client

-------------------------------------------------------------------------------------------------
July 2016. SONOFF client new release 1.2 

Auto-recovery for wifi was added. Also fallback to a second Access Point (SSI) was added.
The node will try to connect to the main SSI for 20 seconds, then try the second SSI and so on.
If no second SSI is present fill in the same SSI-data for SSI_A and SSI_B.

-------------------------------------------------------------------------------------------------
July 2016. SONOFF client new release 1.1 

Client software was adapted to include the node number in MQTT clientID. This is to prevent issues when using multiple nodes.
An MQTT "will" was added to indicate nodes disconnecting.

-------------------------------------------------------------------------------------------------
June 2016. SONOFF client added.

Client software was adapted to function on a commercial hardware platform: sonoff by Itead. This yields a small and cheap unit to switch mains power. More info can be found in the wiki.

-------------------------------------------------------------------------------------------------

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

