#include <SPI.h>
#include <LoRa.h>

#define ss 18
#define rst 14
#define dio0 26

String message;

byte localAddress = 0xFF;  
byte destination = 0xBB;

void setup() {

  Serial.begin(9600);
  while (!Serial);
  //Wire.begin();
  Serial.println("*Juan*");

  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(ss, rst, dio0);  
  if (!LoRa.begin(868E6)) {
    Serial.println("Start of LoRa failed!");
    while (1);
  }
  delay(1000);
}

void loop() {
  while (Serial.available()) {
    delay(2);  //delay to allow byte to arrive in input buffer
    char c = Serial.read();
    message += c;
  }

  if (message.length() > 0) {
    Serial.println("Juan: " + message); //name seen in Serial Monitor
    LoRa.beginPacket();
    LoRa.write(destination);            
    LoRa.write(localAddress);
    LoRa.print("Juan: " + message); //name seen on the receiving end
    LoRa.endPacket();
    message = "";
  }

  onReceive(LoRa.parsePacket());

}


void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  int recipient = LoRa.read();
  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (recipient != localAddress && recipient != 0xBB) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  Serial.print(incoming);
  Serial.print(" || RSSI: ");
  Serial.println(LoRa.packetRssi());
  Serial.println();
}