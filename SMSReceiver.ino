/*
  Arduino SMS Receiver (and sender)
  
  Arduino based simple test code to send and receive SMS messages using SIM800L module.

  Whenever button connected to pin 8 is pressed this code sends message to configured phone numbers.

  Whenever control message is received from authorized phone numbers, leds are turned on or off depending on the command. 
  
  created   Nov 2017
  by CheapskateProjects

  ---------------------------
  The MIT License (MIT)

  Copyright (c) 2017 CheapskateProjects

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>

// Pin definitions
#define FONA_RX 11
#define FONA_TX 10
#define FONA_RST 4
#define LED1 5
#define LED2 6
#define BUTTON1 8

// Message buffers
char smsBuffer[255];
char senderBuffer[255];

// Numbers for send and receive
char allowedNumbers[1][14] =
{
    "+311234567890"
};

// Fona software serial
SoftwareSerial FSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *FSerial = &FSS;

// Fona
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);


void setup()
{
  // Set action pin modes
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1, LOW);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED2, LOW);

  // Wait for serial
  while (!Serial);

  // Init serial 
  Serial.begin(115200);

  // Init module serial
  Serial.println("Initializing serial connection to module");
  FSerial->begin(9600);
  if(!fona.begin(*FSerial))
  {
    Serial.println("Init ERROR: Could not find SIM module!");
    while (1);
  }

  //Give the module some time so that SMS has initialized before trying to read messages
  Serial.println("Waiting for the module to fully initialize...");
  fona.type();
  delay(30000);
  fona.getNumSMS();
  delay(10000);
  Serial.println("Init done");
}


void loop()
{
  // Handle button press
  if(digitalRead(BUTTON1) == LOW)
  {
    // Button was pressed. We should send message to all recipients
    int size = sizeof allowedNumbers / sizeof allowedNumbers[0];
    Serial.println("Send text messages to ");
    for(int i = 0; i < size; i = i + 1)
    {
      Serial.println(allowedNumbers[i]);
      fona.sendSMS(allowedNumbers[i], "Button pressed");
    }

    // Just to give some time to release the button. Otherwise button may be pressend when we reach here again.
    delay(2000);
  }

  // If we have SMS messages they are handled
  int8_t SMSCount = fona.getNumSMS();
  if (SMSCount > 0)
  {
        Serial.println("Found SMS messages!");
        uint16_t smslen;
        for (int8_t smsIndex = 1 ; smsIndex <= SMSCount; smsIndex++)
        {
          Serial.print("Handling message ");
          Serial.println(smsIndex);

          // Read message and check error situations
          if (!fona.readSMS(smsIndex, smsBuffer, 250, &smslen))
          {
            Serial.println("Error: Failed to read message");
            continue;
          }
          if (smslen == 0)
          {
            Serial.println("Error: Empty message");
            continue;
          }

          // Read sender number so that we can check if it was authorized
          fona.getSMSSender(smsIndex, senderBuffer, 250);

          // Compare against each authorized number
          int size = sizeof allowedNumbers / sizeof allowedNumbers[0];
          for( int ind = 0; ind < size; ind = ind + 1)
          {
            if(strstr(senderBuffer,allowedNumbers[ind]))
            {

              // Authorized -> Handle action
              Serial.println("Allowed number found");
              if(strstr(smsBuffer, "LED1 ON"))
              {
                Serial.println("SET: LED1 ON");
                digitalWrite(LED1, HIGH);
              }
              else if(strstr(smsBuffer, "LED1 OFF"))
              {
                Serial.println("SET: LED1 OFF");
                digitalWrite(LED1, LOW);
              }
              else if(strstr(smsBuffer, "LED2 ON"))
              {
                Serial.println("SET: LED2 ON");
                digitalWrite(LED2, HIGH);
              }
              else if(strstr(smsBuffer, "LED2 OFF"))
              {
                Serial.println("SET: LED2 OFF");
                digitalWrite(LED2, LOW);
              }
              else
              {
                Serial.println("Not a known message type");
                Serial.println(smsBuffer);
              }

              // Already found authorized and did the action so we can bail out now
              break;
            }
          }

          // Message was handled or it was unauthorized -> delete
          fona.deleteSMS(smsIndex);

          // Flush buffers
          while (Serial.available())
          {
            Serial.read();
          }
          while (fona.available())
          {
            fona.read();
          }
        }
  }
}


