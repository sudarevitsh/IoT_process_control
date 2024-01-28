Control of process based on IoT technologies (Thesis project)

An industrial line is simulated by a set of 3 ESP8266 MCUs. 
An operator can control the line through the main access point, access the web page and enter their desired input (representing an automated manufactruing process). 

1. MCU represents the central device, acting as an access point, hosting a web server and processing the user inputs (ap_server).
2. MCU collects data from several sensors - the data is used for feedback loop (sensor_client).
3. MCU controlls a set of relay of 7 relays that simulate different machines on the line (line_client).

