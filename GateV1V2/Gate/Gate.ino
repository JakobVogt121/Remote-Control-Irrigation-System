#include <SPI.h>
#include <LoRa.h>

#define ss 18
#define rst 14
#define dio0 26

byte gateAddress = 0xFF;
byte valve1Address = 0xBB;

// Structure LoRa Messages
struct LoRaMsg {
  byte receiver;
  byte transmitter;
  String msg;
};

LoRaMsg msgOut = {
  valve1Address,
  gateAddress,
  ""
};

// 1. device struct mit Adresse und Name
// 2. Array aus devices: in Schleife Empfängeradresse vergleichen und Empfängernamen nennen

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
  Serial.println("Gate ready");
}

void loop() {
  while (Serial.available()) {
    delay(2);  //delay to allow byte to arrive in input buffer
    char c = Serial.read();
    msgOut.msg += c;
  }

  if (msgOut.msg.length() > 0) {
    Serial.println("Gate sends: " + msgOut.msg);
    //Serial.println("to " + valveX);
    LoRa.beginPacket();
    LoRa.write(msgOut.receiver);
    LoRa.write(msgOut.transmitter);
    LoRa.print("Gate: " + msgOut.msg);
    LoRa.endPacket();
    msgOut.msg = "";
  }

  onReceive(LoRa.parsePacket());
}


void onReceive(int packetSize) {
  if (packetSize == 0) return;  // if there's no packet, return

  int target = LoRa.read();
  int transmitter = LoRa.read();

  String incoming = "";
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (target != gateAddress) {
    //Serial.println("This message is not for me.");
    return;  // skip rest of function
  }

  if (transmitter == valve1Address) {
    Serial.println("Message from Valve1:");
  } else {
    Serial.println(transmitter, HEX);
  }

  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print(incoming);
  Serial.print(" || RSSI: ");
  Serial.println(LoRa.packetRssi());
  Serial.println();
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}