# Smart-Alarm-Clock-2810
Our goal is to create an alarm clock to help people who travel a lot wake up with enough time to get to their flight but also get a few extra minutes of sleep when flights are delayed.

# Setup
This project uses an ESP32 microcontroller.
You can upload this code using the ArduinoIDE.

You will need 4 buttons, a 16x2 LCD display, a buzzer, a resistor, a transistor, and an ESP32 extension board (Allows for battery power safely, I believe it also has built in resistors for the LCD pins).

## Pin Setup
Left Button -> GPIO 4
Right Button -> GPIO 5
Select Button -> GPIO 13
Cycle Screen Button -> GPIO 18
LCD SDA -> GPIO 21
LCD SCL -> GPIO 22
Buzzer -> GPIO 14
Buzzer -> 3.3V
LCD -> 5V

Pin 14 connects to a resister rated at 1K Ohms which connects to the middle leg of the transistor.
The transistor's left leg is connected to ground and it's right leg is connected to the negative side of our buzzer.

The buttons are also all connected to ground as well as the LCD display.
