# Lidar_laser

Will be used to scan the floor plan of the apartment, right now he's only able to display lines on the python `reciever`.

Using both the cpu cores of the `poe` we can recieve a lot of information and at the same time send those in `local net` -> zZzzzZ

The only commands that the Lidar responded to are the `STOP`and `START` with the `CHANGE SPEED` it seems to have no effect **:C**

## Cable connection

https://www.olimex.com/Products/IoT/ESP32/ESP32-POE/resources/ESP32-POE-GPIO.png

1. Black -> 5V (USB)
2. Green -> GND (USB)
3. Red -> TX (POE) GPIO 4
4. White -> RX (POE) GPIO 36

`It's easier to just connect the black and green to the usb power source than the POE`
