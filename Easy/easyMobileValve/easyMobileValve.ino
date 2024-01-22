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

#define pinValve 32

byte valve3Address = 3;
byte stateCockCrow = 9;
bool rx2byteMsg = false;
byte rxValve = 0;
byte rxValveState = 0;

void setup() {
  //pinMode(LED_BUILTIN, OUTPUT);  // for debugging/visualization
  esp_sleep_enable_timer_wakeup(5000000);

  pinMode(pinValve, OUTPUT);
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

  for (int i = 1; i <= 3; i++) {    
    cockCrow();
    if (rx2byteMsg == true) {
      i = 3;
    }
    LoRa.receive();
    delay(500); // random delay zwischen .. und .. einführen
  }
  if (rx2byteMsg == true) {
    Serial.println("rxValve: ");
    Serial.println(rxValve);
    Serial.println("rxValveState: ");
    Serial.println(rxValveState);
    if (rxValve == valve3Address) {
      Serial.println("V3");
      digitalWrite(pinValve, rxValveState);
      confirm_cmd(rxValve, rxValveState);
      delay(3000);
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