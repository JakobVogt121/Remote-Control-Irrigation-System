/*==========================================================================================
                                          easyValves
  ==========================================================================================*/
  
/* Description
Erhält Daten über LoRa von easyGate:
  10: Ventil 1 aus
  11: Ventil 1 an
  20: Ventil 2 aus
  21: Ventil 2 an
und sendet Bestätigung über LoRa an easyGate zurück:
  10: Ventil 1 aus
  11: Ventil 1 an
  20: Ventil 2 aus
  21: Ventil 2 an
*/

#include <LoRa.h>

#define ss 18
#define rst 14
#define dio0 26

#define pinV1 32
#define pinV2 33

byte valve1Address = 1;
byte valve2Address = 2;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);  // for debugging/visualization
  pinMode(pinV1, OUTPUT);
  pinMode(pinV2, OUTPUT);
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
  Serial.println("Valve 1, 2 ready");
}

void loop() {
  onLoRaReceive(LoRa.parsePacket());
}

void onLoRaReceive(int packetSize) {
  //digitalWrite(LED_BUILTIN, HIGH);
  if (packetSize == 2) {
    byte rxValve = LoRa.read();
    byte rxValveState = LoRa.read();

    Serial.println("rxValve: ");
    Serial.println(rxValve);
    Serial.println("rxValveState: ");
    Serial.println(rxValveState);

    if (rxValve == valve1Address) {
      Serial.println("V1");
      digitalWrite(pinV1, rxValveState);
      confirm_cmd(rxValve, rxValveState);
    } else if (rxValve == valve2Address) {
      Serial.println("V2");
      digitalWrite(pinV2, rxValveState);
      confirm_cmd(rxValve, rxValveState);
    } else {
      Serial.println("not for me");
      return;  // skip rest of function
    }
    //digitalWrite(LED_BUILTIN, LOW);
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