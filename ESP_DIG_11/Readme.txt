ESP-DIG Release 1.1 

by Computourist@gmail.com May 2016

- some minor bug fixing and code improvement:

	- setting of toggle/timer and Ack function had a bug
	- mqttsubs code improved
	- device 40 implemented
	- in timer function a message is now generated on state change



==============================================================================================================
ESP-DIG Release 1.0 

by Computourist@gmail.com May 2016

The DIG end node is basically a lean node, based on a ESP8266 Wifi Module. It was tested with an ESP-01 board.


It's only function is to control a digital output (Relay, LED), it has:
- an output to control the load (set at pin 0)
- an input to connect a pushbutton (at pin 2 with a pull-up resistor of 10 K)

The push button can be used to locally toggle the output. On every state change a message can be generated.

Connection to the MQTT broker is over Wifi. Topics used are:
home/esp_gw/nb/nodexx/devyy for messages from node to broker
home/esp_gw/sb/nodexx/devyy for messages from broker to node.

nodexx is the node ID, fixed and unique for every node.
devyy is a device on a node; devices for this node are:

0	uptime:			read uptime node in minutes
1	node:			read/set transmission interval in seconds, 0 means no periodic transmission
3	Version:		read version node software
5	ACK:			read/set acknowledge message after a 'set' request
6	toggle:			read/set toggle function on button press
7	timer:			read/set activation timer after button press in seconds, 0 means no timer
16	actuator:		read/set LED or relay output

91	error:			tx only: syntax error message 
92	error:			tx only: device not supported
99	wakeup:			tx only: first message sent on node startup


