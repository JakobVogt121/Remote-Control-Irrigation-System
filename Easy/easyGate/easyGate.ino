/*==========================================================================================
                                          easyGate
  ==========================================================================================*/

/* Description
Erhält von Node-Red über Serielle Schnittstelle:
  10: Ventil 1 aus
  11: Ventil 1 an
  20: Ventil 2 aus
  21: Ventil 2 an
  30: Ventil 3 aus
  31: Ventil 3 an
und sendet die Daten über LoRa weiter an easyValves.
Bekommt Daten von easyValves über LoRa zurück und sendet diese wieder über Serielle Schnittstelle an Node-Red
  10: Ventil 1 aus
  11: Ventil 1 an
  20: Ventil 2 aus
  21: Ventil 2 an
  30: Ventil 3 aus
  31: Ventil 3 an
Besonderheit mobile Valve 3:
die Kommunikation initiiert das mobile Ventil, sobald es aus dem Schlaf erwacht. Es fragt seinen Status durch:
  39: CockCrow (3x)
an. Das Gate  hört das Hahnenkrähen und schickt es (39) 1x zu Node-Red weiter.
*/

#include <LoRa.h>

// Lora Pin Settings
#define ss 18
#define rst 14
#define dio0 26

//const byte gateAddress = 0;
// receiver Valve
byte rxValve = 0;
byte rxValveState;
// transmitter/sender Valve
byte txValve;
byte txValveState;
byte txValveLastState = 0;
byte txValveCurrentState = 0;
bool txValveFlag = false;  // to register cockCrow of txValve only once
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
  //Serial.println("Gate ready");
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
    LoRa.receive();
  }

  if (txValveFlag == true) {
    send2RPI();
  }
}

void onReceive(int packetSize) {

  if (packetSize == 2) {
    // gateAddress not used to check if message is for gate
    txValve = LoRa.read();
    txValveState = LoRa.read();
    txValveCurrentState = 10 * txValve + txValveState;
    if (txValveCurrentState != txValveLastState) {
      txValveFlag = true;
    }
    txValveLastState = txValveCurrentState;
  } else {
    return;  // if there's no packet or a packet with a wrong size, return
  }
}

void send2RPI() {
  Serial.print(txValve);
  Serial.print(txValveState);
  Serial.println();
  txValveFlag = false;
}