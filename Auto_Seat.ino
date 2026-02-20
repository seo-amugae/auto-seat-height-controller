//
// last modify  2026-02-20
// Priority: ignition status -> parking status -> seat position
//
#include <EEPROM.h>

#define seatRelayD8 8   // HIGH = OFF, LOW = ON (상승 릴레이)
#define seatRelayD9 9   // HIGH = OFF, LOW = ON (하강 릴레이)
#define parkingPin A0   // 기어 PARKING 전압값 (RAW, 0~1023)
#define ignitionPin 7   // HIGH = IG1 ON, LOW = IG1 OFF

boolean ignitionState = LOW;
boolean parkingState = LOW;

float currentPos = 0.0;
float marginTime = 500.0;

unsigned long lastTime = 0;

void setup() {
  pinMode(parkingPin, INPUT);
  pinMode(ignitionPin, INPUT);

  pinMode(seatRelayD8, OUTPUT);
  pinMode(seatRelayD9, OUTPUT);

  seat(0);

  if (EEPROM.read(102) == 1) {
    currentPos = 5500.0;
    marginTime = 500.0;
  } else {
    currentPos = 0.0;
    marginTime = 500.0;
  }

  lastTime = millis();
}

void loop() {
  measurement();

  unsigned long currentTime = millis();
  unsigned long deltaTime = currentTime - lastTime;
  lastTime = currentTime;

  bool wantUp = (ignitionState == HIGH && parkingState == LOW);

  if (wantUp) {
    if (currentPos < 5500.0) {
      seat(1);
      currentPos += deltaTime;

      if (currentPos >= 5500.0) {
        currentPos = 5500.0;
        marginTime = 0.0;
      }
    } else {
      if (marginTime < 500.0) {
        seat(1);
        marginTime += deltaTime;
      } else {
        marginTime = 500.0;
        seat(0);
        EEPROM.update(102, 1);
      }
    }
  } else {
    if (currentPos > 0.0) {
      seat(-1);
      currentPos -= deltaTime * 1.1;

      if (currentPos <= 0.0) {
        currentPos = 0.0;
        marginTime = 0.0;
      }
    } else {
      if (marginTime < 500.0) {
        seat(-1);
        marginTime += deltaTime;
      } else {
        marginTime = 500.0;
        seat(0);
        EEPROM.update(102, 0);
      }
    }
  }
}

void setRelay(int relayPin, boolean state) {
  digitalWrite(relayPin, state ? LOW : HIGH);
}

void seat(int action) {
  if (action == 1) { // 상승
    setRelay(seatRelayD8, 1);
    setRelay(seatRelayD9, 0);
  } else if (action == -1) { // 하강
    setRelay(seatRelayD8, 0);
    setRelay(seatRelayD9, 1);
  } else { // 정지
    setRelay(seatRelayD8, 0);
    setRelay(seatRelayD9, 0);
  }
}

void measurement() {
  ignitionState = digitalRead(ignitionPin);
  parkingState = analogRead(parkingPin) >= 950 ? HIGH : LOW;
}
