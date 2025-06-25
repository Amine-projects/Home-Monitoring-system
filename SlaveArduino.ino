#include <Wire.h>
#include <Servo.h>

// Pin definitions
const int greenLedPin = 3;
const int redLedPin = 4;
const int servoPin = 12;

// System state
bool systemArmed = false;
bool alarmActive = false;

// Servo control
Servo lockServo;
const int LOCKED_POSITION = 0;
const int UNLOCKED_POSITION = 90;

void setup() {
  // Initialize I2C as slave with address 8
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  
  // Initialize serial for debugging
  Serial.begin(9600);
  Serial.println("Home Security System {Slave}");
  
  // Set up pins
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  
  // Initialize servo
  lockServo.attach(servoPin);
  lockServo.write(UNLOCKED_POSITION); // Start in unlocked position
  
  // Set initial LED states - green on (system disarmed), red off
  digitalWrite(greenLedPin, HIGH);
  digitalWrite(redLedPin, LOW);
}

void loop() {
  // If in alarm mode, oscillate the servo to simulate alarm
  if (alarmActive) {
    // Oscillate servo between positions
    lockServo.write(LOCKED_POSITION);
    delay(500);
    lockServo.write(UNLOCKED_POSITION);
    delay(500);
  } else {
    // Make sure servo is in the correct position based on system state
    if (systemArmed) {
      lockServo.write(LOCKED_POSITION);
    } else {
      lockServo.write(UNLOCKED_POSITION);
    }
  }
  
  delay(100); // Small delay for stability
}

// Function called when slave receives data from master
void receiveEvent(int howMany) {
  char command = Wire.read();
  Serial.print("Received command: ");
  Serial.println(command);
  
  switch (command) {
    case 'A': // Arm the system
      systemArmed = true;
      alarmActive = false;
      
      // Green LED off, red LED on when armed
      digitalWrite(greenLedPin, LOW);
      digitalWrite(redLedPin, HIGH);
      
      // Lock the servo
      lockServo.write(LOCKED_POSITION);
      
      Serial.println("System ARMED");
      break;
      
    case 'D': // Disarm the system
      systemArmed = false;
      alarmActive = false;
      
      // Green LED on, red LED off when disarmed
      digitalWrite(greenLedPin, HIGH);
      digitalWrite(redLedPin, LOW);
      
      // Unlock the servo
      lockServo.write(UNLOCKED_POSITION);
      
      Serial.println("System DISARMED");
      break;
      
    case 'B': // Breach detected - activate alarm
      alarmActive = true;
      activateAlarm();
      break;
  }
}

// Function to activate the alarm
void activateAlarm() {
  // Green LED off, red LED on during alarm
  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, HIGH);
  
  // Start servo oscillation (will be handled in main loop)
  Serial.println("ALARM ACTIVATED!");
}