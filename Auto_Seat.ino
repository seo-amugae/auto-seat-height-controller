//
// last modify  2025-09-30 04:00:00
// last upload  2025-10-04 05:00:00
//
// Priority
// seat position -> ignition status -> parking status
//
#include <EEPROM.h>
#define ADDRESS 9

#define seatRelayD8 8   // HIGH = OFF, LOW = ON
#define seatRelayD9 9   // HIGH = OFF, LOW = ON
#define parkingPin A0   // 기어 PARKING IND. 전압값 (RAW, 0~1023)
#define ignitionPin 7   // HIGH = IG1 ON, LOW = IG1 OFF

boolean ignitionState = 0, seatPosition = 0, parkingState = 0;

void setup() {
  pinMode(parkingPin, INPUT);
  pinMode(ignitionPin, INPUT);

  pinMode(seatRelayD8, OUTPUT);
  pinMode(seatRelayD9, OUTPUT);

  setRelay(seatRelayD8, 0);
  setRelay(seatRelayD9, 0);

  seatPosition = EEPROM.read(ADDRESS);
}

void loop() {
  measurement();

  if ((seatPosition && parkingState) || (seatPosition && !ignitionState)) {
    seat(-1);
    unsigned long startTime = millis();
    while (millis() - startTime <= 5500) {
      measurement();
      if (ignitionState && !parkingState) {
        seat(1);
        delay(millis() - startTime + 1000);
        seat(0);
        return;
      }
    }
    seat(0);
    seatPosition = 0;
    EEPROM.update(ADDRESS, 0);
  } else if (!seatPosition && ignitionState && !parkingState) {
    unsigned long stabilizationStart = millis();
    while (millis() - stabilizationStart <= 500) {
      measurement();
      if (parkingState) return;
    }
    seat(1);
    unsigned long startTime = millis();
    while (millis() - startTime <= 6000) {
      measurement();
      if (parkingState || !ignitionState) {
        seat(-1);
        delay(millis() - startTime + 500);
        seat(0);
        return;
      }
    }
    seat(0);
    seatPosition = 1;
    EEPROM.update(ADDRESS, 1);
  }
}


void setRelay(int relayPin, boolean state) {
  digitalWrite(relayPin, state ? LOW : HIGH);
}

void seat(int action) {
  if (action == 1) {
    setRelay(seatRelayD8, 1);
    setRelay(seatRelayD9, 0);
  } else if (action == 0) {
    setRelay(seatRelayD8, 0);
    setRelay(seatRelayD9, 0);
  } else if (action == -1) {
    setRelay(seatRelayD8, 0);
    setRelay(seatRelayD9, 1);
  }
}

void measurement() {
  ignitionState = digitalRead(ignitionPin);
  parkingState = analogRead(parkingPin) >= 987 ? HIGH : LOW;
}
