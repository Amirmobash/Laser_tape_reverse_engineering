#include <LiquidCrystal_I2C.h>

const byte ENTRY_SENSOR_PIN = 2;
const byte EXIT_SENSOR_PIN = 3;

LiquidCrystal_I2C lcd(0x27, 16, 2);

int currentPeople = 0;
int totalEntries = 0;
int totalExits = 0;

const int MAX_PEOPLE = 999;
const unsigned long DEBOUNCE_TIME = 200;
const unsigned long SENSOR_TIMEOUT = 500;
const unsigned long DISPLAY_INTERVAL = 500;

enum CounterState {
  WAITING,
  ENTRY_DETECTED,
  EXIT_DETECTED
};

CounterState counterState = WAITING;

bool lastEntryState = HIGH;

unsigned long lastSensorTime = 0;
unsigned long lastDisplayTime = 0;

void setup() {
  Serial.begin(9600);

  pinMode(ENTRY_SENSOR_PIN2, INPUT_PULLUP);
  pinMode(EXIT_SENSOR_PIN2, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  showStartupScreen();
  updateDisplay();

  Serial.println("People counter is ready");
  Serial.println("Send R to reset counters");
  Serial.println("Send S to show current status");
}

void loop() {
  readSensors();
  updateDisplayIfNeeded();
  handleSerialInput();

  delay(10);
}

void readSensors() {
  bool entryState = digitalRead(ENTRY_SENSOR_PIN);
  bool exitState = digitalRead(EXIT_SENSOR_PIN);

  bool entryTriggered = lastEntryState == HIGH && entryState == LOW;
  bool exitTriggered = lastExitState == HIGH && exitState == LOW;

  switch (counterState) {
    case WAITING:
      if (entryTriggered) {
        counterState = ENTRY_DETECTED;
        lastSensorTime = millis();
      } else if (exitTriggered) {
        counterState = EXIT_DETECTED;
        lastSensorTime = millis();
      }
      break;

    case ENTRY_DETECTED:
      if (exitTriggered && isWithinSensorTime()) {
        addPerson();
        resetSensorState();
      } else if (hasSensorTimedOut()) {
        resetSensorState();
      }
      break;

    case EXIT_DETECTED:
      if (entryTriggered && isWithinSensorTime()) {
        removePerson();
        resetSensorState();
      } else if (hasSensorTimedOut()) {
        resetSensorState();
      }
      break;
  }

  lastEntryState = entryState;
  lastExitState = exitState;
}

bool isWithinSensorTime() {
  return millis() - lastSensorTime < SENSOR_TIMEOUT;
   Serial.println("Send S to show current status");
}

bool hasSensorTimedOut() {
  return millis() - lastSensorTime >= SENSOR_TIMEOUT;
}

void resetSensorState() {
  counterState = WAITING;
  delay(DEBOUNCE_TIME);
}

void addPerson() {
  if (currentPeople < MAX_PEOPLE) {
    currentPeople++;
    totalEntries++;

    Serial.print("[ENTRY] One person entered. Current: ");
    Serial.print(currentPeople);
    Serial.print(" | In: ");
    Serial.print(totalEntries);
    Serial.print(" | Out: ");
    Serial.println(totalExits);
  }
}

void removePerson() {
  if (currentPeople > 0) {
    currentPeople--;
    totalExits++;

    Serial.print("[EXIT] One person left. Current: ");
    Serial.print(currentPeople);
    Serial.print(" | In: ");
    Serial.print(totalEntries);
    Serial.print(" | Out: ");
    Serial.println(totalExits);
  } else {
    Serial.println("[WARNING] Exit detected while room is already empty");
  }
}

void updateDisplayIfNeeded() {
  if (millis() - lastDisplayTime >= DISPLAY_INTERVAL) {
    updateDisplay();
    lastDisplayTime = millis();
  }
}

void updateDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("IN:");
  printNumberAt(3, 0, totalEntries, 3);

  lcd.setCursor(8, 0);
  lcd.print("OUT:");
  printNumberAt(12, 0, totalExits, 3);

  lcd.setCursor(0, 1);
  lcd.print("People:");
  printNumberAt(8, 1, currentPeople, 3);

  lcd.setCursor(15, 1);
  if (currentPeople >= 10) {
    lcd.print("!");
  } else {
    lcd.print(" ");
  }
}

void printNumberAt(byte column, byte row, int number, byte width) {
  lcd.setCursor(column, row);

  for (byte i = 0; i < width; i++) {
    lcd.print(" ");
  }

  lcd.setCursor(column, row);

  if (number < 10 && width >= 2) lcd.print(" ");
  if (number < 100 && width >= 3) lcd.print(" ");

  lcd.print(number);
}

void showStartupScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("People Counter");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");
  delay(1200);
  lcd.clear();
}

void resetCounters() {
  currentPeople = 0;
  totalEntries = 0;
  totalExits = 0;

  updateDisplay();

  Serial.println("[RESET] All counters have been reset");
}

void printStatus() {
  Serial.print("[STATUS] Current: ");
  Serial.print(currentPeople);
  Serial.print(" | In: ");
  Serial.print(totalEntries);
  Serial.print(" | Out: ");
  Serial.println(totalExits);
}

void handleSerialInput() {
  if (!Serial.available()) {
    return;
  }

  char command = Serial.read();

  if (command == 'r' || command == 'R') {
    resetCounters();
  } else if (command == 's' || command == 'S') {
    printStatus();
  }
}
