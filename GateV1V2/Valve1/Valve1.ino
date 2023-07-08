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
  String msg;
};

LoRaMsg msgOut = {
  gateAddress,
  valve1Address,
  ""
};

//String message;

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
  while (Serial.available()) {
    delay(2);  //delay to allow byte to arrive in input buffer
    char c = Serial.read();
    msgOut.msg += c;
  }

  if (msgOut.msg.length() > 0) {
    Serial.println("Valve1 sends to Gate: " + msgOut.msg);
    LoRa.beginPacket();
    LoRa.write(msgOut.receiver);
    LoRa.write(msgOut.transmitter);
    LoRa.print("Valve1: " + msgOut.msg);  //name seen on the receiving end
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

  if (target != valve1Address) {
    //Serial.println("This message is not for me.");
    return;  // skip rest of function
  }

  // Msg immer vom Gate
  /*if (transmitter == gateAddress) {
    Serial.println("Message from gate:");
  } else {
    Serial.println(transmitter, HEX);
  }*/

  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print(incoming);
  Serial.print(" || RSSI: ");
  Serial.println(LoRa.packetRssi());
  Serial.println();
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}