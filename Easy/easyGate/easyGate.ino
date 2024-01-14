/*==========================================================================================
                                          easyGate
  ==========================================================================================*/

/* Description
Erhält von Node-Red über Serielle Schnittstelle:
  10: Ventil 1 aus
  11: Ventil 1 an
  20: Ventil 2 aus
  21: Ventil 2 an
und sendet die Daten über LoRa weiter an easyValves.
Bekommt Daten von easyValves über LoRa zurück und sendet diese wieder über Serielle Schnittstelle an Node-Red
  10: Ventil 1 aus
  11: Ventil 1 an
  20: Ventil 2 aus
  21: Ventil 2 an
*/

#include <LoRa.h>

// Lora Pin Settings
#define ss 18
#define rst 14
#define dio0 26

//const byte gateAddress = 0;
//const byte valve1Address = 1;
//const byte valve2Address = 2;

// receiver Valve
byte rxValve = 0;
byte rxValveState;
// transmitter/sender Valve
byte txValve;
byte txValveState;
bool txValveFlag = false; // to register cockCrow of txValve only once
// --> gets raised, when State of sender Valve changes

void setup() {
  // Serial Communication (debugging)
  Serial.begin(9600);
  while (!Serial)
    ;
  // LoRa Communication
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(868E6)) {
    //Serial.println("Start of LoRa failed!");
    while (1)
      ;
  }
  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT);  // for debugging/visualization
  Serial.println("Gate ready");

  LoRa.receive();
  // Register a callback function for when a packet is received using interrupt pin on the dio0
  LoRa.onReceive(onReceive);
}

void loop() {

  while (Serial.available()) {
    delay(2);  //delay to allow byte to arrive in input buffer
    char c_rxValve = Serial.read();
    rxValve = c_rxValve - '0';
    char c_rxValveState = Serial.read();
    rxValveState = c_rxValveState - '0';
    delay(10);
    /*Serial.println("Gate sends: ");
    Serial.print("Receiver Valve: ");
    Serial.print(rxValve);
    Serial.print("\nrxValveState: ");
    Serial.println(rxValveState);*/

    LoRa.beginPacket();
    LoRa.write(rxValve);
    LoRa.write(rxValveState);
    LoRa.endPacket();
  }
  //(onReceive);
  //onLoRaReceive(LoRa.parsePacket());
  if (txValveFlag == true) {
    Serial.print(txValve);
    Serial.print(txValveState);
    Serial.println();

    Serial.print(3);
    Serial.print(1);
    Serial.println();

    LoRa.beginPacket();
    LoRa.write(3);
    LoRa.write(1);
    LoRa.endPacket();

    txValveFlag = false;
  }
  //Serial.print(txValve);
  //Serial.print(txValveState);
  //Serial.println();
}

void onReceive(int packetSize) {

  if (packetSize == 2) {
    // gateAddress not used to check if message is for gate
    txValve = LoRa.read();
    txValveState = LoRa.read();
    txValveFlag = true;

    //Serial.println("Received from Valves: ");
    //Serial.print(target);
    //Serial.print(state);
    //Serial.println();

    //Serial.print("RSSI: ");
    //Serial.println(LoRa.packetRssi());
    //delay(1000);
    //digitalWrite(LED_BUILTIN, LOW);
  } else {
    return;  // if there's no packet or a packet with a wrong size, return
  }
}