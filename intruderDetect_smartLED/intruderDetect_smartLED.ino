#include <IRremote.h>

const unsigned long interval = 300000; // 5 minutes in milliseconds
unsigned long previousMillis = 0;
unsigned long currentTime = 0;

const int ldrPin = A0;       // Analog pin connected to LDR
const int motionPin = 12;     // Digital pin connected to motion sensor OUT
const int ledPin = 13;        // Digital pin connected to LED
const int threshold = 200;   // Light level threshold 


const int RECV_PIN = 11; // Pin connected to the IR receiver module
IRrecv irrecv(RECV_PIN);
decode_results results;

const int hallSensorPin = 2;   // Hall sensor output connected to digital pin 2
const int buzzerPin = 3;       // Buzzer connected to digital pin 3

unsigned long disableTime = 0; // Variable to keep track of the disable time

void setup() {
  pinMode(motionPin, INPUT);   
  pinMode(ledPin, OUTPUT);     
  pinMode(hallSensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(9600);

  irrecv.enableIRIn(); // Start the IR receiver
}

void loop() {
  int lightLevel = analogRead(ldrPin);  // Read light level from LDR
  int motionDetected = digitalRead(motionPin);  // Read motion sensor


  // Check if light level is below the threshold and motion is detected
  if (lightLevel < threshold && motionDetected == HIGH) {
    digitalWrite(ledPin, HIGH);  // Turn on the LED
    delay(3000); // Keep the LED on for 3 second
  } else {
    digitalWrite(ledPin, LOW);   // Turn off the LED
  }

  // Read the state of the Hall sensor
  int sensorValue = digitalRead(hallSensorPin);

  if (irrecv.decode(&results)) {
    Serial.print("Received value: 0x");
    Serial.println(results.value, HEX);
    
    // Check if the received value is the desired one
    if (results.value == 0xFFFFFFFF) { 
      Serial.println("stop the buzzer/hall sensor in 2 minutes");
      disableTime = millis() + 120000; // Set disable time to 2 minutes from now
    }
    
    irrecv.resume(); // Receive the next value
  }

  // Check if the current time is past the disable time
  if (millis() > disableTime) {
    // If Hall sensor does not detect magnet (High state or 1), sound the buzzer
    if (sensorValue == HIGH) {
      currentTime = millis() + 300001;
      if (currentTime - previousMillis >= interval) {
        Serial.println("SendAlertMail");
        previousMillis = currentTime; // Update the previous time
      }

      digitalWrite(buzzerPin, HIGH);  // Turn on the buzzer
      delay(1000);                    // Buzzer sounds for 1 second
      digitalWrite(buzzerPin, LOW);   // Turn off the buzzer
      delay(1000);                    // Wait for 1 second 
    }
  }
}



