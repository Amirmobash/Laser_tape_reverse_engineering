#include <LiquidCrystal_I2C.h>

const byte entrySensor = 2;
const byte exitSensor = 3;

LiquidCrystal_I2C lcd(0x27, 16, 2);

int people = 0;
int entered = 0;
int left = 0;

const int limit = 9999;
const unsigned long waitTime = 500;
const unsigned long pauseTime = 200;
const unsigned long screenTime = 200;

enum State {
  Ready,
  EntrySeen,
  ExitSeen
};

State state = Ready;

bool lastEntry = HIGH;
bool lastExit = HIGH;

unsigned long firstSeenTime = 0;
unsigned long lastCountTime = 0;
unsigned long lastScreenTime = 0;

void setup() {
  Serial.begin(9600);

  pinMode(entrySensor, INPUT_PULLUP);
  pinMode(exitSensor, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  startScreen();
  showNumbers();

  Serial.println("People counter started");
  Serial.println("Type R to reset");
  Serial.println("Type S to see the numbers");
}

void loop() {
  checkSensors();
  refreshScreen();
  readCommand();

  delay(10);
}

void checkSensors() {
  bool entryNow = digitalRead(entrySensor);
  bool exitNow = digitalRead(exitSensor);

  bool entryTouched = lastEntry == HIGH && entryNow == LOW;
  bool exitTouched = lastExit == HIGH && exitNow == LOW;

  if (millis() - lastCountTime < pauseTime) {
    lastEntry = entryNow;
    lastExit = exitNow;
    return;
  }

  if (state == Ready) {
    if (entryTouched) {
      state = EntrySeen;
      firstSeenTime = millis();
    } else if (exitTouched) {
      state = ExitSeen;
      firstSeenTime = millis();
    }
  } else if (state == EntrySeen) {
    if (exitTouched && millis() - firstSeenTime <= waitTime) {
      personEntered();
      resetSensors();
    } else if (millis() - firstSeenTime > waitTime) {
      resetSensors();
    }
  } else if (state == ExitSeen) {
    if (entryTouched && millis() - firstSeenTime <= waitTime) {
      personLeft();
      resetSensors();
    } else if (millis() - firstSeenTime > waitTime) {
      resetSensors();
    }
  }

  lastEntry = entryNow;
  lastExit = exitNow;
}

void resetSensors() {
  state = Ready;
  lastCountTime = millis();
}

void personEntered() {
  if (people < limit) {
    people++;
    entered++;

    showNumbers();

    Serial.print("Someone entered. People inside: ");
    Serial.print(people);
    Serial.print(" | Entered: ");
    Serial.print(entered);
    Serial.print(" | Left: ");
    Serial.println(left);
  }
}

void personLeft() {
  if (people > 0) {
    people--;
    left++;

    showNumbers();

    Serial.print("Someone left. People inside: ");
    Serial.print(people);
    Serial.print(" | Entered: ");
    Serial.print(entered);
    Serial.print(" | Left: ");
    Serial.println(left);
  } else {
    Serial.println("Someone tried to leave, but nobody is inside");
  }
}

void refreshScreen() {
  if (millis() - lastScreenTime >= screenTime) {
    showNumbers();
    lastScreenTime = millis();
  }
}

void showNumbers() {
  lcd.setCursor(0, 0);
  lcd.print("In:");
  printValue(3, 0, entered, 4);

  lcd.setCursor(8, 0);
  lcd.print("Out:");
  printValue(12, 0, left, 4);

  lcd.setCursor(0, 1);
  lcd.print("People:");
  printValue(8, 1, people, 4);

  lcd.setCursor(15, 1);
  if (people >= 10) {
    lcd.print("!");
  } else {
    lcd.print(" ");
  }
}

void printValue(byte column, byte row, int value, byte width) {
  lcd.setCursor(column, row);

  for (byte i = 0; i < width; i++) {
    lcd.print(" ");
  }

  lcd.setCursor(column, row);

  if (value < 10 && width >= 2) lcd.print(" ");
  if (value < 100 && width >= 3) lcd.print(" ");
  if (value < 1000 && width >= 4) lcd.print(" ");

  lcd.print(value);
}

void startScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("People Counter");
  lcd.setCursor(0, 1);
  lcd.print("Ready to Count");
  delay(1200);
  lcd.clear();
}

void resetCounter() {
  people = 0;
  entered = 0;
  left = 0;

  showNumbers();

  Serial.println("All numbers are reset");
}

void showStatus() {
  Serial.print("People inside: ");
  Serial.print(people);
  Serial.print(" | Entered: ");
  Serial.print(entered);
  Serial.print(" | Left: ");
  Serial.println(left);
}

void readCommand() {
  if (!Serial.available()) {
    return;
  }

  char command = Serial.read();

  if (command == 'R' || command == 'r') {
    resetCounter();
  } else if (command == 'S' || command == 's') {
    showStatus();
  }
}
