/*==========================================================================================
                                          easyValves
  ==========================================================================================*/
  
/* Description
Erhält Daten über LoRa von easyGate:
  10: Ventil 1 aus
  11: Ventil 1 an
  20: Ventil 1 aus
  21: Ventil 1 an
und sendet Bestätigung über LoRa an easyGate zurück:
  10: Ventil 1 aus
  11: Ventil 1 an
  20: Ventil 1 aus
  21: Ventil 1 an
*/

#include <LoRa.h>

#define ss 18
#define rst 14
#define dio0 26

#define pinV1 32
#define pinV2 33

byte gateAddress = 0;
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
  Serial.println("Valves ready");
}

void loop() {
  onLoRaReceive(LoRa.parsePacket());
}

void onLoRaReceive(int packetSize) {
  //digitalWrite(LED_BUILTIN, HIGH);
  if (packetSize == 2) {
    byte target = LoRa.read();
    byte state = LoRa.read();

    Serial.println("Target: ");
    Serial.println(target);
    Serial.println("State: ");
    Serial.println(state);

    if (target == valve1Address) {
      Serial.println("V1");
      digitalWrite(pinV1, state);
      confirm_cmd(target, state);
    } else if (target == valve2Address) {
      Serial.println("V2");
      digitalWrite(pinV2, state);
      confirm_cmd(target, state);
    } else {
      Serial.println("Error");
      return;  // skip rest of function
    }
    //delay(1000);
    //digitalWrite(LED_BUILTIN, LOW);
    //Serial.print("RSSI: ");
    //Serial.println(LoRa.packetRssi());
  } else {
    return;  // if there's no packet or a packet with a wrong size, return
  }
}

void confirm_cmd(byte target, byte state) {
  delay(1000);
  Serial.print(target);
  Serial.print(state);
  Serial.println();
  LoRa.beginPacket();
  LoRa.write(target);
  LoRa.write(state);
  LoRa.endPacket();
}