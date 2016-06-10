ESP-BAS Release 1.0 


by Computourist@gmail.com June 2016

The BAS end node is a minimalistic node, based on a ESP8266 Wifi Module. It was tested with an ESP-01 board.

Its only function is to read a digital input and control a digital output (Relay, LED), it has:
- an output to control the load (set at pin 0)
- an input to connect a switch or PIR-sensor (at pin 2 with a pull-up resistor of 10 K)

On every state change of the input a message is generated. 
The output can be controlled by sending a control message.
Periodically the state of input and output is sent. 

Connection to the MQTT broker is over Wifi. Topics used are:
home/esp_gw/nb/nodexx/devyy for messages from node to broker
home/esp_gw/sb/nodexx/devyy for messages from broker to node.

nodexx is the node ID, fixed and unique for every node.
devyy is a device on a node; devices for this node are:

0	uptime:			read uptime node in minutes
1	node:			read/set transmission interval in seconds, 0 means no periodic transmission
3	Version:		read version node software
16	actuator:		read/set LED or relay output

91	error:			tx only: syntax error message 
92	error:			tx only: device not supported
99	wakeup:			tx only: first message sent on node startup


