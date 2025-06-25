#include <Wire.h>
#include <Keypad.h>

// Keypad setup
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Security code storage
char securityCode[5] = "";  // 4 digits + null terminator
char enteredCode[5];
int codeIndex = 0;

// Sensor and button
const int hallSensorPin = 9;
const int buttonPin = 10;

// System state
bool systemArmed = false;
bool breachDetected = false;
bool awaitingButtonPress = false;
bool codeSetupMode = true;
bool alarmActive = false;

// Timing variables
unsigned long codeEnteredTime = 0;
unsigned long alarmStartTime = 0;
const unsigned long buttonTimeLimit = 5000; // 5 seconds

// Transition after arming/disarming
bool systemTransitioning = false;
unsigned long transitionStartTime = 0;
const unsigned long transitionDuration = 3000; // 3 seconds

void setup() {
  Wire.begin();
  Serial.begin(9600);
  delay(1000);

  pinMode(hallSensorPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.println("Home Security System");
  Serial.println("SETUP MODE: Enter 4-digit security code, then * to confirm.");
}

void loop() {
  // Handle transition delay after arming/disarming
  if (systemTransitioning && millis() - transitionStartTime > transitionDuration) {
    systemTransitioning = false;
    Serial.println("System stable, monitoring resumed.");
  }

  // Setup mode
  if (codeSetupMode) {
    handleCodeSetup();
    return;
  }

  // Active alarm
  if (alarmActive) {
    Wire.beginTransmission(8);
    Wire.write('B');
    Wire.endTransmission();

    if (digitalRead(buttonPin) == LOW) {
      deactivateAlarm();
      Serial.println("Alarm deactivated by button.");
      Serial.println("Enter your code to re-arm the system.");
    }
  }

  // Awaiting button press after code
  else if (awaitingButtonPress) {
    if (digitalRead(buttonPin) == LOW) {
      awaitingButtonPress = false;
      systemArmed = !systemArmed;

      Serial.print("Button pressed! System ");
      Serial.println(systemArmed ? "ARMED" : "DISARMED");

      breachDetected = false;
      alarmActive = false;

      Wire.beginTransmission(8);
      Wire.write(systemArmed ? 'A' : 'D');
      Wire.endTransmission();

      systemTransitioning = true;
      transitionStartTime = millis();
    }

    if (millis() - codeEnteredTime > buttonTimeLimit) {
      awaitingButtonPress = false;
      activateAlarm();
    }
  }

  // Normal operation
  else if (!systemTransitioning) {
    handleKeypadInput();

    if (systemArmed && !breachDetected) {
      int sensorValue = digitalRead(hallSensorPin);
      Serial.print("Hall sensor reading: ");
      Serial.println(sensorValue);

      if (sensorValue == HIGH) {
        breachDetected = true;
        activateAlarm();
        Serial.println("BREACH DETECTED AT YOUR ROOM DOOR!");
      }
    }
  }

  delay(100);
}

void handleCodeSetup() {
  char key = keypad.getKey();
  if (key) {
    Serial.print(key);

    if (key == '#') {
      codeIndex = 0;
      Serial.println("\nCode cleared.");
    } 
    else if (key == '*') {
      if (codeIndex == 4) {
        securityCode[codeIndex] = '\0';
        codeSetupMode = false;
        codeIndex = 0;
        Serial.println("\nSecurity code set!");
        Serial.println("Re-enter to arm the system");
      } else {
        Serial.println("\nPlease enter exactly 4 digits!");
        codeIndex = 0;
      }
    }
    else if (codeIndex < 4) {
      securityCode[codeIndex++] = key;
    }
  }
}

void handleKeypadInput() {
  char key = keypad.getKey();
  if (key) {
    Serial.print(key);

    if (key == '#') {
      codeIndex = 0;
      Serial.println("\nCode cleared.");
    } 
    else if (key == '*') {
      enteredCode[codeIndex] = '\0';
      if (strcmp(enteredCode, securityCode) == 0) {
        Serial.println("\nCorrect code! Press button within 5 seconds.");
        awaitingButtonPress = true;
        codeEnteredTime = millis();
      } else {
        Serial.println("\nIncorrect code!");
      }
      codeIndex = 0;
    } 
    else if (codeIndex < 4) {
      enteredCode[codeIndex++] = key;
    }
  }
}

void activateAlarm() {
  alarmActive = true;
  breachDetected = true;
  alarmStartTime = millis();

  Serial.println("ALARM ACTIVATED!");
  Serial.println("Press button to deactivate and try again.");
}

void deactivateAlarm() {
  alarmActive = false;
  breachDetected = false;
  systemArmed = false;

  Wire.beginTransmission(8);
  Wire.write('D');
  Wire.endTransmission();
}
