#include <LiquidCrystal_I2C.h>

const int IR_SENSOR_IN = 2;
const int IR_SENSOR_OUT = 3;

LiquidCrystal_I2C lcd(0x27, 16, 2);

int personCount = 0;
int totalIn = 0;
int totalOut = 0;

const unsigned long DEBOUNCE_DELAY = 200;
const unsigned long SEQUENCE_TIMEOUT = 500;

enum SensorState {
  IDLE,
  IN_TRIGGERED,
  OUT_TRIGGERED
};

SensorState currentState = IDLE;
unsigned long lastTriggerTime = 0;
unsigned long lastDisplayUpdate = 0;

bool lastInState = HIGH;
bool lastOutState = HIGH;

void setup() {
  Serial.begin(9600);
  
  lcd.init();
  lcd.backlight();
  
  pinMode(IR_SENSOR_IN, INPUT_PULLUP);
  pinMode(IR_SENSOR_OUT, INPUT_PULLUP);
  
  updateDisplay();
  
  delay(1000);
  Serial.println("System gestartet - Personenzahler bereit");
}

void loop() {
  bool inState = digitalRead(IR_SENSOR_IN);
  bool outState = digitalRead(IR_SENSOR_OUT);
  
  bool inFalling = (lastInState == HIGH && inState == LOW);
  bool outFalling = (lastOutState == HIGH && outState == LOW);
  
  switch(currentState) {
    case IDLE:
      if (inFalling) {
        currentState = IN_TRIGGERED;
        lastTriggerTime = millis();
      }
      else if (outFalling) {
        currentState = OUT_TRIGGERED;
        lastTriggerTime = millis();
      }
      break;
      
    case IN_TRIGGERED:
      if (outFalling && (millis() - lastTriggerTime) < SEQUENCE_TIMEOUT) {
        personIncrement();
        currentState = IDLE;
        delay(DEBOUNCE_DELAY);
      }
      else if (millis() - lastTriggerTime >= SEQUENCE_TIMEOUT) {
        currentState = IDLE;
      }
      break;
      
    case OUT_TRIGGERED:
      if (inFalling && (millis() - lastTriggerTime) < SEQUENCE_TIMEOUT) {
        personDecrement();
        currentState = IDLE;
        delay(DEBOUNCE_DELAY);
      }
      else if (millis() - lastTriggerTime >= SEQUENCE_TIMEOUT) {
        currentState = IDLE;
      }
      break;
  }
  
  if ((millis() - lastDisplayUpdate) >= 500) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
  
  lastInState = inState;
  lastOutState = outState;
  
  processSerialCommands();
  
  delay(10);
}

void personIncrement() {
  if (personCount < 999) {
    personCount++;
    totalIn++;
    
    Serial.print("[EINTRITT] Person betritt Raum - Aktuell: ");
    Serial.print(personCount);
    Serial.print(" (IN: ");
    Serial.print(totalIn);
    Serial.print(", OUT: ");
    Serial.print(totalOut);
    Serial.println(")");
  }
}

void personDecrement() {
  if (personCount > 0) {
    personCount--;
    totalOut++;
    
    Serial.print("[AUSTRITT] Person verl?sst Raum - Aktuell: ");
    Serial.print(personCount);
    Serial.print(" (IN: ");
    Serial.print(totalIn);
    Serial.print(", OUT: ");
    Serial.print(totalOut);
    Serial.println(")");
  } else {
    Serial.println("[WARNUNG] Austritt ohne vorherigen Eintritt!");
  }
}

void updateDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("IN:");
  lcd.setCursor(3, 0);
  lcd.print("   ");
  lcd.setCursor(3, 0);
  if (totalIn < 10) lcd.print(" ");
  if (totalIn < 100) lcd.print(" ");
  lcd.print(totalIn);
  
  lcd.setCursor(9, 0);
  lcd.print("OUT:");
  lcd.setCursor(13, 0);
  lcd.print("   ");
  lcd.setCursor(13, 0);
  if (totalOut < 10) lcd.print(" ");
  if (totalOut < 100) lcd.print(" ");
  lcd.print(totalOut);
  
  lcd.setCursor(0, 1);
  lcd.print("Personen:");
  lcd.setCursor(9, 1);
  lcd.print("    ");
  lcd.setCursor(9, 1);
  if (personCount < 10) lcd.print(" ");
  if (personCount < 100) lcd.print(" ");
  lcd.print(personCount);
  
  if (personCount >= 10) {
    lcd.setCursor(15, 1);
    lcd.print("!");
  } else {
    lcd.setCursor(15, 1);
    lcd.print(" ");
  }
}

void resetCounters() {
  personCount = 0;
  totalIn = 0;
  totalOut = 0;
  updateDisplay();
  Serial.println("[RESET] Alle Zahler wurden zuruckgesetzt");
}

void processSerialCommands() {
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'r' || cmd == 'R') {
      resetCounters();
    }
    else if (cmd == 's' || cmd == 'S') {
      Serial.print("Status - Aktuell: ");
      Serial.print(personCount);
      Serial.print(", IN: ");
      Serial.print(totalIn);
      Serial.print(", OUT: ");
      Serial.println(totalOut);
    }
  }
}
