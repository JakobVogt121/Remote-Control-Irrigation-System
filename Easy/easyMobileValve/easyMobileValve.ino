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
  10: Ventil 1 aus
  11: Ventil 1 an
  20: Ventil 2 aus
  21: Ventil 2 an
*/

#include <LoRa.h>

#define ss 18
#define rst 14
#define dio0 26

#define pinValve 32

//byte gateAddress = 0;
byte valve3Address = 3;

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
    Serial.println("Start of LoRa failed!");
    while (1)
      ;
  }
  delay(1000);
  Serial.println("mobile Valve 3 ready");
  // Register a callback function for when a packet is received using interrupt pin on the dio0
  LoRa.onReceive(onReceive);
}

void loop() {
  for (int i = 0; i <= 10; i++) {
    cockCrow();
    LoRa.receive();
    delay(500);
  }
  esp_deep_sleep_start();
}

void cockCrow() {
  Serial.print(3);
  Serial.print(9);
  Serial.println();
  LoRa.beginPacket();
  LoRa.write(3);  // valve 3
  LoRa.write(9);  // state 9 = advertising
  LoRa.endPacket();
}

void onReceive(int packetSize) {
  //digitalWrite(LED_BUILTIN, HIGH);
  if (packetSize == 2) {
    byte rxValve = LoRa.read();
    byte rxValveState = LoRa.read();

    Serial.println("rxValve: ");
    Serial.println(rxValve);
    Serial.println("rxValveState: ");
    Serial.println(rxValveState);

    if (rxValve == valve3Address) {
      Serial.println("V3");
      digitalWrite(pinValve, rxValveState);
      confirm_cmd(rxValve, rxValveState);
      esp_deep_sleep_start();
    } else {
      Serial.println("Error");
      return;  // skip rest of function
    }
  } else {
    return;  // if there's no packet or a packet with a wrong size, return
  }
}

void confirm_cmd(byte rxValve, byte rxValveState) {
  delay(500);
  Serial.print(rxValve);
  Serial.print(rxValveState);
  Serial.println();
  LoRa.beginPacket();
  LoRa.write(rxValve);
  LoRa.write(rxValveState);
  LoRa.endPacket();
}