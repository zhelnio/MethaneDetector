# MethaneDetector

This natural gas detector was created for using in the kitchen where a gas oven is used. It is based on MQ-4 gas sensor.
According to the sensor datasheet the standard detection conditions includes Vcc=5V±0.1 and preheat time over 24 hours. The natural gas pressure in the centralized gas supply system is very small (we don't use a gas tank). Thats why there is no need to measure the gas concentration continuously. According to this the gas sensor in this detector is working not continuous.
The typical working cycle includes: 1 minute of preheat, concentration measurements and 5 minutes of waiting. These parameters can be easily changed in the firmware source code.
For the device simplification the measurement circuit is directly connected to the ATtiny port, so the voltage on it is about 4.4V, not 5V. These conditions (maybe) reduce the MQ-4 sensitivity but it still can be used as gas detector.

**Schematic diagram**
![Alt text](/readme/schema.png?raw=true "schema")

**Board layout**

![Alt text](/readme/board.png?raw=true "board")

**Device photo**

![Alt text](/readme/photo.JPG?raw=true "photo")
