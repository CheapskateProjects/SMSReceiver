# SMSReceiver
Arduino based simple test code to send and receive SMS messages using SIM800L module.
Whenever button connected to pin 8 is pressed this code sends message to configured phone numbers.
Whenever control message is received from authorized phone numbers, leds are turned on or off depending on the command.

Dependency(Adafruit FONA library): <https://github.com/adafruit/Adafruit_FONA>
