
ESP-DIG Release 1.0 

by Computourist@gmail.com May 2016

The DIG end node is basically a lean node, based on a ESP8266 Wifi Module. It was tested with an ESP-01 board.


It's only function is to control a digital output (Relay, LED), it has:
- an output to control the load (set at pin 0)
- an input to connect a pushbutton (at pin 2 with a pull-up resistor of 10 K)

The push button can be used to locally toggle the output. On every state change a message can be generated.

Connection to the MQTT broker is over Wifi. 

 Defined devices are:

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


