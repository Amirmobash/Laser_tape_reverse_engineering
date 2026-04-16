#include <LiquidCrystal_I2C.h>

// Pin-Definitionen
const int IR_SENSOR_IN = 2;   // Eingangssensor (betreten)
const int IR_SENSOR_OUT = 3;  // Ausgangssensor (verlassen)

// LCD-Konfiguration
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Zähler
int personCount = 0;      // Aktuelle Personenanzahl
int totalIn = 0;          // Gesamt eingetreten
int totalOut = 0;         // Gesamt ausgetreten

// Entprellung
const unsigned long DEBOUNCE_DELAY = 200;  // ms
const unsigned long SEQUENCE_TIMEOUT = 500; // ms für Bewegungssequenz

// Zustände
enum SensorState {
  IDLE,
  IN_TRIGGERED,
  OUT_TRIGGERED
};

SensorState currentState = IDLE;
unsigned long lastTriggerTime = 0;
unsigned long lastDisplayUpdate = 0;

// Status der Sensoren (für Flankenerkennung)
bool lastInState = HIGH;
bool lastOutState = HIGH;

void setup() {
  Serial.begin(9600);
  
  // LCD initialisieren
  lcd.init();
  lcd.backlight();
  
  // Sensoren mit Pull-up konfigurieren
  pinMode(IR_SENSOR_IN, INPUT_PULLUP);
  pinMode(IR_SENSOR_OUT, INPUT_PULLUP);
  
  // LCD-Header anzeigen
  updateDisplay();
  
  delay(1000);
  Serial.println(F("System gestartet - Personenzähler bereit"));
}

void loop() {
  // Sensoren auslesen
  bool inState = digitalRead(IR_SENSOR_IN);
  bool outState = digitalRead(IR_SENSOR_OUT);
  
  // Flankenerkennung für beide Sensoren
  bool inFalling = (lastInState == HIGH && inState == LOW);
  bool outFalling = (lastOutState == HIGH && outState == LOW);
  
  // Zustandsmaschine für Personenerkennung
  switch(currentState) {
    case IDLE:
      if (inFalling) {
        // Erster Sensor (Eingang) ausgelöst
        currentState = IN_TRIGGERED;
        lastTriggerTime = millis();
      }
      else if (outFalling) {
        // Erster Sensor (Ausgang) ausgelöst
        currentState = OUT_TRIGGERED;
        lastTriggerTime = millis();
      }
      break;
      
    case IN_TRIGGERED:
      // Warte auf zweiten Sensor innerhalb des Timeouts
      if (outFalling && (millis() - lastTriggerTime) < SEQUENCE_TIMEOUT) {
        // Person betritt den Raum
        personIncrement();
        currentState = IDLE;
        delay(DEBOUNCE_DELAY); // Zusätzliche Entprellung
      }
      else if (millis() - lastTriggerTime >= SEQUENCE_TIMEOUT) {
        // Timeout - falsche Auslösung
        currentState = IDLE;
      }
      break;
      
    case OUT_TRIGGERED:
      // Warte auf zweiten Sensor innerhalb des Timeouts
      if (inFalling && (millis() - lastTriggerTime) < SEQUENCE_TIMEOUT) {
        // Person verlässt den Raum
        personDecrement();
        currentState = IDLE;
        delay(DEBOUNCE_DELAY); // Zusätzliche Entprellung
      }
      else if (millis() - lastTriggerTime >= SEQUENCE_TIMEOUT) {
        // Timeout - falsche Auslösung
        currentState = IDLE;
      }
      break;
  }
  
  // Display regelmäßig aktualisieren (nur bei Änderung oder alle 500ms)
  if (personCount != getDisplayedCount() || 
      (millis() - lastDisplayUpdate) >= 500) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
  
  // Sensorzustände für nächsten Durchlauf speichern
  lastInState = inState;
  lastOutState = outState;
  
  delay(10); // Kleine Verzögerung für stabile Messung
}

// Person betritt Raum
void personIncrement() {
  if (personCount < 999) { // Überlaufschutz
    personCount++;
    totalIn++;
    
    // Debug-Ausgabe
    Serial.print(F("[EINTRITT] Person betritt Raum - Aktuell: "));
    Serial.print(personCount);
    Serial.print(F(" (IN: "));
    Serial.print(totalIn);
    Serial.print(F(", OUT: "));
    Serial.print(totalOut);
    Serial.println(F(")"));
  }
}

// Person verlässt Raum
void personDecrement() {
  if (personCount > 0) {
    personCount--;
    totalOut++;
    
    // Debug-Ausgabe
    Serial.print(F("[AUSTRITT] Person verlässt Raum - Aktuell: "));
    Serial.print(personCount);
    Serial.print(F(" (IN: "));
    Serial.print(totalIn);
    Serial.print(F(", OUT: "));
    Serial.print(totalOut);
    Serial.println(F(")"));
  } else {
    // Warnung wenn Austritt ohne vorherigen Eintritt
    Serial.println(F("[WARNUNG] Austritt ohne vorherigen Eintritt!"));
  }
}

// Display aktualisieren
void updateDisplay() {
  // Zeile 1: IN und OUT Anzeige
  lcd.setCursor(0, 0);
  lcd.print(F("IN:"));
  lcd.setCursor(3, 0);
  lcd.print("   "); // Löschen
  lcd.setCursor(3, 0);
  if (totalIn < 10) lcd.print(" ");
  if (totalIn < 100) lcd.print(" ");
  lcd.print(totalIn);
  
  lcd.setCursor(9, 0);
  lcd.print(F("OUT:"));
  lcd.setCursor(13, 0);
  lcd.print("   ");
  lcd.setCursor(13, 0);
  if (totalOut < 10) lcd.print(" ");
  if (totalOut < 100) lcd.print(" ");
  lcd.print(totalOut);
  
  // Zeile 2: Aktuelle Personenanzahl
  lcd.setCursor(0, 1);
  lcd.print(F("Personen:"));
  lcd.setCursor(9, 1);
  lcd.print("    "); // Löschen
  lcd.setCursor(9, 1);
  if (personCount < 10) lcd.print(" ");
  if (personCount < 100) lcd.print(" ");
  lcd.print(personCount);
  
  // Visuelle Warnung bei Überfüllung
  if (personCount >= 10) {
    lcd.setCursor(15, 1);
    lcd.print(F("!"));
  } else {
    lcd.setCursor(15, 1);
    lcd.print(F(" "));
  }
}

// Hilfsfunktion für Display-Vergleich
int getDisplayedCount() {
  return personCount; // Vereinfacht - in der Praxis müsste man vom LCD lesen
}

// Optional: Reset-Funktion (kann über seriellen Monitor aufgerufen werden)
void resetCounters() {
  personCount = 0;
  totalIn = 0;
  totalOut = 0;
  updateDisplay();
  Serial.println(F("[RESET] Alle Zähler wurden zurückgesetzt"));
}

// Serielle Befehle verarbeiten (optional)
void processSerialCommands() {
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'r' || cmd == 'R') {
      resetCounters();
    }
    else if (cmd == 's' || cmd == 'S') {
      Serial.print(F("Status - Aktuell: "));
      Serial.print(personCount);
      Serial.print(F(", IN: "));
      Serial.print(totalIn);
      Serial.print(F(", OUT: "));
      Serial.println(totalOut);
    }
  }
}