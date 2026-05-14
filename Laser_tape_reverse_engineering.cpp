#include <LiquidCrystal_I2C.h>

const byte entrySensorPin = 2;
const byte exitSensorPin = 3;

LiquidCrystal_I2C lcd(0x27, 16, 2);

int peopleInside = 0;
int totalIn = 0;
int totalOut = 0;

const int maxPeople = 9999;
const unsigned long sensorTimeLimit = 500;
const unsigned long debounceTime = 200;
const unsigned long displayTime = 200;

enum SensorState {
  Waiting,
  EntryFirst,
  ExitFirst
};

SensorState sensorState = Waiting;

bool lastEntryState = HIGH;
bool lastExitState = HIGH;

unsigned long firstSensorTime = 0;
unsigned long lastCountTime = 0;
unsigned long lastDisplayTime = 0;

void setup() {
  Serial.begin(9600);

  pinMode(entrySensorPin, INPUT_PULLUP);
  pinMode(exitSensorPin, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  showStartScreen();
  updateDisplay();

  Serial.println("People counter is ready");
  Serial.println("Send R to reset");
  Serial.println("Send S to show status");
}

void loop() {
  checkSensors();
  updateDisplayByTime();
  checkSerial();

  delay(10);
}

void checkSensors() {
  bool entryNow = digitalRead(entrySensorPin);
  bool exitNow = digitalRead(exitSensorPin);

  bool entryPressed = lastEntryState == HIGH && entryNow == LOW;
  bool exitPressed = lastExitState == HIGH && exitNow == LOW;

  if (millis() - lastCountTime < debounceTime) {
    lastEntryState = entryNow;
    lastExitState = exitNow;
    return;
  }

  switch (sensorState) {
    case Waiting:
      if (entryPressed) {
        sensorState = EntryFirst;
        firstSensorTime = millis();
      } else if (exitPressed) {
        sensorState = ExitFirst;
        firstSensorTime = millis();
      }
      break;

    case EntryFirst:
      if (exitPressed && sensorIsInTime()) {
        addPerson();
        resetSensors();
      } else if (sensorIsTooLate()) {
        resetSensors();
      }
      break;

    case ExitFirst:
      if (entryPressed && sensorIsInTime()) {
        removePerson();
        resetSensors();
      } else if (sensorIsTooLate()) {
        resetSensors();
      }
      break;
  }

  lastEntryState = entryNow;
  lastExitState = exitNow;
}

bool sensorIsInTime() {
  return millis() - firstSensorTime <= sensorTimeLimit;
}

bool sensorIsTooLate() {
  return millis() - firstSensorTime > sensorTimeLimit;
}

void resetSensors() {
  sensorState = Waiting;
  lastCountTime = millis();
}

void addPerson() {
  if (peopleInside < maxPeople) {
    peopleInside++;

    if (totalIn < maxPeople) {
      totalIn++;
    }

    updateDisplay();

    Serial.print("Entry detected. People: ");
    Serial.print(peopleInside);
    Serial.print(" | In: ");
    Serial.print(totalIn);
    Serial.print(" | Out: ");
    Serial.println(totalOut);
  }
}

void removePerson() {
  if (peopleInside > 0) {
    peopleInside--;

    if (totalOut < maxPeople) {
      totalOut++;
    }

    updateDisplay();

    Serial.print("Exit detected. People: ");
    Serial.print(peopleInside);
    Serial.print(" | In: ");
    Serial.print(totalIn);
    Serial.print(" | Out: ");
    Serial.println(totalOut);
  } else {
    Serial.println("Exit detected, but the room is empty");
  }
}

void updateDisplayByTime() {
  if (millis() - lastDisplayTime >= displayTime) {
    updateDisplay();
    lastDisplayTime = millis();
  }
}

void updateDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("In:");
  printNumber(3, 0, totalIn, 4);

  lcd.setCursor(8, 0);
  lcd.print("Out:");
  printNumber(12, 0, totalOut, 4);

  lcd.setCursor(0, 1);
  lcd.print("People:");
  printNumber(8, 1, peopleInside, 4);

  lcd.setCursor(15, 1);
  if (peopleInside >= 10) {
    lcd.print("!");
  } else {
    lcd.print(" ");
  }
}

void printNumber(byte column, byte row, int number, byte width) {
  lcd.setCursor(column, row);

  for (byte i = 0; i < width; i++) {
    lcd.print(" ");
  }

  lcd.setCursor(column, row);

  if (number < 10 && width >= 2) lcd.print(" ");
  if (number < 100 && width >= 3) lcd.print(" ");
  if (number < 1000 && width >= 4) lcd.print(" ");

  lcd.print(number);
}

void showStartScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("People Counter");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");
  delay(1200);
  lcd.clear();
}

void resetCounter() {
  peopleInside = 0;
  totalIn = 0;
  totalOut = 0;

  updateDisplay();

  Serial.println("Counter reset");
}

void showStatus() {
  Serial.print("People: ");
  Serial.print(peopleInside);
  Serial.print(" | In: ");
  Serial.print(totalIn);
  Serial.print(" | Out: ");
  Serial.println(totalOut);
}

void checkSerial() {
  if (!Serial.available()) {
    return;
  }

  char command = Serial.read();

  if (command == 'R' || command == 'r') {
    resetCounter();
  }

  if (command == 'S' || command == 's') {
    showStatus();
  }
}
