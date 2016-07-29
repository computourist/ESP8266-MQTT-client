ESP-SONOFF Release 1.2 


by Computourist@gmail.com July 2016

Added support for fallback SSI and autorecovery of wifi link.

node will try first SSI for 20 seconds. If no connection is possible it will fall back the to second SSI.

==============================================================================================================
ESP-SONOFF Release 1.1 

by Computourist@gmail.com July 2016

two major changes:

- The MQTT client-id is made node specific by including the node number: ESP_20.
A clientID must be unique for the broker. This change prevents multiple clients with identical clientID.

- An MQTT "will" has been included to indicate whenever a node gets disconnected. 
The clientID will then be published in topic: home/esp_gw/disconnected

==============================================================================================================
ESP-SONOFF Release 1.0 

by Computourist@gmail.com June 2016

The SONOFF end node is a small an cheap power switch node, based on a ESP8266 Wifi Module.


Its  function is to control a relay and switch mains power.

It has:
- an output to control a green LED (set at GPIO 13)
- an output to control the load (set at GPIO 12)
- an input to connect a pushbutton (at GPIO 0)

A dual color LED is included in the unit, where the red LED is not used. The unit can be modified to have the red LED light up when the load is switched on.

The green LED will shiow the following sequence:

- it wil light up for a second when the WIFI connection is established.
- it will go out for one second
- it wil light up and stay lit indicating an active MQTT link.

The push button can be used to locally toggle the output and als to initiate memory flash mode.

For details on programming and altering the LED circuit look at the wiki at:
 https://github.com/computourist/ESP8266-MQTT-client/wiki 

Connection to the MQTT broker is over Wifi. Topics used are:
- home/esp_gw/nb/nodexx/devyy for messages from node to broker
- home/esp_gw/sb/nodexx/devyy for messages from broker to node.

	nodexx is the node ID, fixed and unique for every node.
	devyy is a device on a node; 

devices for this node are:

0	uptime:			read uptime node in minutes
1	node:			read/set transmission interval in seconds, 0 means no periodic transmission
3	Version:		read version node software
5	ACK:			read/set acknowledge message after a 'set' request
6	toggle:			read/set toggle function on button press
7	timer:			read/set activation timer after button press in seconds, 0 means no timer
10	IP address:		read currently aasigned IP address
16	actuator:		read/set LED or relay output

91	error:			tx only: syntax error message 
92	error:			tx only: device not supported
99	wakeup:			tx only: first message sent on node startup


