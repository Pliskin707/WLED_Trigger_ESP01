# WLED_Trigger_ESP01
Simple ESP-01 controller with some buttons to trigger WLED effects via the JSON API

# What does it do?

## Serial flash
When flashed with the non-OTA build, the ESP will output information via the serial0 (baud = 115200). 
The program requests some information from the WLED controller, prints it out and toggles to/from the WLED preset 3 every 10 seconds. 

No user interaction is possible.

## Over-The-Air (OTA) flash
When flashed over the air (requires at least one serial flash before OTA is possible), the device uses the build in LED to display the WiFi connection state. Other than that no output is provided. 
The program requests some information from the WLED controller and stores it for later operation.

User interaction is possible with some buttons on GPIO0 and GPIO1 (active low, pull up). If a pin gets shorted to ground, WLED preset 4 or 5 (depending on the pin) gets activated for three seconds. Afterwards the previously stored values get restored.
