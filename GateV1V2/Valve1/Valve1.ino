#include <SPI.h>
#include <LoRa.h>

#define ss 18
#define rst 14
#define dio0 26

byte valve1Address = 0xBB;
byte gateAddress = 0xFF;

// Structure LoRa Messages
struct LoRaMsg {
  byte receiver;
  byte transmitter;
  bool state;
};

LoRaMsg msgOut = {
  gateAddress,
  valve1Address,
  false
};

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
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
  Serial.println("Valve1 ready");
}

void loop() {
  onReceive(LoRa.parsePacket());
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;  // if there's no packet, return

  byte target = LoRa.read();
  byte transmitter = LoRa.read();
  bool state = LoRa.read();

  /*
  Serial.println(target,HEX);
  Serial.println(transmitter,HEX);
  Serial.println((char)state ? "TRUE" : "FALSE");  
  */

  if (target != valve1Address) {
    return;  // skip rest of function
  }
  
  digitalWrite(LED_BUILTIN, state);
  //Serial.print("RSSI: ");
  //Serial.println(LoRa.packetRssi());
  // acknowledge message
  confirm_cmd(state);
}

void confirm_cmd(bool state) {
  msgOut.state = state;
  delay(10);
  Serial.print("Valve1 sends: ");
  Serial.println((msgOut.state) ? "TRUE" : "FALSE");
  Serial.println();
  LoRa.beginPacket();
  LoRa.write(msgOut.receiver);
  LoRa.write(msgOut.transmitter);
  LoRa.write(msgOut.state);
  LoRa.endPacket();
}