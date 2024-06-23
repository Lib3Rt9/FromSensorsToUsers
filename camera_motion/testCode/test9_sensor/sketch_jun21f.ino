// If you're using the built-in LED for testing, remember that it works with inverted logic
// (HIGH=LOW, and LOW=HIGH)
int led = 13;               // the pin that the LED is attached to
int sensor = 12;            // the pin that the sensor is attached to
int state = LOW;            // by default, no motion detected
int val = 0;                // variable to store the sensor status (value)

void setup() {
  pinMode(led, OUTPUT);     // initialize LED as an output
  pinMode(sensor, INPUT);   // initialize sensor as an input
  Serial.begin(9600);       // initialize serial communication
}

void loop() {
  val = digitalRead(sensor);    // read sensor value
  if (val == HIGH) {            // check if the sensor is HIGH
    digitalWrite(led, HIGH);    // turn LED ON
    
    if (state == LOW) {
      Serial.println("Motion detected!"); 
      state = HIGH;             // update variable state to HIGH
    }
  } 
  else {
    digitalWrite(led, LOW);     // turn LED OFF
    
    if (state == HIGH) {
      Serial.println("Motion stopped!");
      state = LOW;              // update variable state to LOW
    }
  }

  // Print the sensor value to serial
  Serial.print("Sensor Value: ");
  Serial.println(val);

  delay(500);  // delay for stability
}
