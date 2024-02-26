/*==========================================================================================
                                          easyValves
  ==========================================================================================*/

/* Description
- schläft
- wacht auf
- fordert Ventilstatus durch:
  39
  über LoRa von Gate/Node-Red an
- erhält Daten über LoRa von easyGate:
  30: Ventil 3 aus
  31: Ventil 3 an
- sendet Bestätigung über LoRa an easyGate zurück:
  30: Ventil 3 aus
  31: Ventil 3 an
*/

#include <LoRa.h>

#define ss 18
#define rst 14
#define dio0 26

#define uS_TO_S_FACTOR 1000000  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 10        // Time ESP32 will go to sleep (in seconds)

#define pinValveOn 32
#define pinValveOff 33
const int moveValveDelay = 3000;  // dleay to wait until valve has been opened/closed

byte valve3Address = 3;            // address of this mobile valve
byte stateCockCrow = 9;            // Cock Crow State
const int numCockCrow = 3;         // Number of Cock Crows
bool rx2byteMsg = false;           // received 2 byte message
byte rxValve = 0;                  // receiving vavle (addressed from gate to a valve, not neccessarily this valve)
byte rxValveState = 0;             // received valve state: 0 closed, 1 opened
RTC_DATA_ATTR byte lastState = 0;  // laste state of this valve: 0 closed, 1 opened

void setup() {
  //pinMode(LED_BUILTIN, OUTPUT);  // for debugging/visualization
  esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * TIME_TO_SLEEP);

  pinMode(pinValveOn, OUTPUT);
  pinMode(pinValveOff, OUTPUT);

  digitalWrite(pinValveOn, LOW);
  digitalWrite(pinValveOff, LOW);

  // Serial nur für debugging
  Serial.begin(9600);
  while (!Serial)
    ;
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(868E6)) {
    //Serial.println("Start of LoRa failed!");
    while (1)
      ;
  }
  delay(1000);
  LoRa.receive();
  Serial.println("mobile Valve 3 ready");
  // Register a callback function for when a packet is received using interrupt pin on the dio0
  LoRa.onReceive(onReceive);
}

void loop() {

  for (int i = 1; i <= numCockCrow; i++) {
    cockCrow();
    if (rx2byteMsg == true && rxValve == valve3Address) {
      break; // if to byte message for vavlve 3 has already been received
    }
    LoRa.receive();
    delay(500);  // random delay zwischen .. und .. einführen
  }
  if (rx2byteMsg == true) {
    Serial.println("rxValve: ");
    Serial.println(rxValve);
    Serial.println("rxValveState: ");
    Serial.println(rxValveState);
    if (rxValve == valve3Address) {
      if (rxValveState != lastState) {
        if (rxValveState == 1) {
          digitalWrite(pinValveOn, HIGH);
        } else if (rxValveState == 0) {
          digitalWrite(pinValveOff, HIGH);
        }
      }
      Serial.println("V3");
      confirm_cmd(rxValve, rxValveState);
      delay(moveValveDelay);
      lastState = rxValveState;
      esp_deep_sleep_start();
    } else {
      Serial.println("not for me");
    }
    rx2byteMsg = false;
  }
  esp_deep_sleep_start();
}

void cockCrow() {
  Serial.print(valve3Address);
  Serial.print(stateCockCrow);
  Serial.println();
  LoRa.beginPacket();
  LoRa.write(valve3Address);  // valve 3
  LoRa.write(stateCockCrow);  // state 9 = advertising
  LoRa.endPacket();
}

void onReceive(int packetSize) {
  //digitalWrite(LED_BUILTIN, HIGH);
  if (packetSize == 2) {
    rxValve = LoRa.read();
    rxValveState = LoRa.read();
    rx2byteMsg = true;
  } else {
    return;  // if there's no packet or a packet with a wrong size, return
  }
}

void confirm_cmd(byte rxValve, byte rxValveState) {
  delay(100);
  Serial.print(rxValve);
  Serial.print(rxValveState);
  Serial.println();
  LoRa.beginPacket();
  LoRa.write(rxValve);
  LoRa.write(rxValveState);
  LoRa.endPacket();
}