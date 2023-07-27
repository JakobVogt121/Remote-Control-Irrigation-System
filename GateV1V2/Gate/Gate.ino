/*-------------------------------
  Gate
  -------------------------------*/
// Description

#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <LoRa.h>
#include <ArduinoJson.h>

// Lora Pin Settings
#define ss 18
#define rst 14
#define dio0 26


/*-------------------------------
    Wifi and MQTT Settings 
  -------------------------------*/
// Wifi
const char* ssid = "Vodafone-93E6";
const char* password = "JFtEaFTgbhcpCRUG";
const char* mqtt_server = "192.168.0.236";  // static Address of MQTT Broker
WiFiClient espClient;
PubSubClient client(espClient);

/*-------------------------------
    Irrigation System Parameters 
  -------------------------------*/
// Adresses
byte gateAddress = 0xFF;
byte valve1Address = 0xBB;

// Structure LoRa Messages
struct LoRaMsg {
  byte receiver;
  byte transmitter;
  bool stateValve;
};

LoRaMsg msgOut = {
  valve1Address,
  gateAddress,
  false
};

// 1. device struct mit Adresse und Name
// 2. Array aus devices: in Schleife Empfängeradresse vergleichen und Empfängernamen nennen

void setup() {
  // Serial Communication (debugging)
  Serial.begin(9600);
  while (!Serial)
    ;

  // LoRa Communication
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(868E6)) {
    Serial.println("Start of LoRa failed!");
    while (1)
      ;
  }

  // WiFi and MQTT Connection
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  delay(1000);

  // setu up Builtin LED for Debugging
  pinMode(LED_BUILTIN, OUTPUT);
  // Gate ready
  Serial.println("Gate ready");
}

void loop() {
  /*while (Serial.available()) {
    delay(2);  //delay to allow byte to arrive in input buffer
    char c = Serial.read();
    //Serial.println(c);
    msgOut.stateValve = (c == '1');
    delay(10);
    Serial.print("Gate sends: ");
    Serial.println((msgOut.stateValve) ? "TRUE" : "FALSE");
    //Serial.println("to Valve X");
    LoRa.beginPacket();
    LoRa.write(msgOut.receiver);
    LoRa.write(msgOut.transmitter);
    LoRa.write(msgOut.stateValve);
    LoRa.endPacket();
  }*/

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  /*long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    char buffer[256];
    serializeJson(doc, buffer);
    //  serializeJsonPretty(doc, Serial);
    client.publish("esp32/temperature", buffer);
*/
  //LoRaonReceive(LoRa.parsePacket());
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);  // connecting to a WiFi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Erklärung Serial.print("Message arrived on topic: ");
void callback(char* topic, byte* message, unsigned int length) {
  Serial.println("callback");
  // Json Document
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message, length);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  // Fetch values.
  //
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do doc["time"].as<long>();
  const byte Address = doc["Address"];
  bool state = doc["state"];

  // Print values.
  Serial.println(Address);
  Serial.println((state) ? "TRUE" : "FALSE");

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      //digitalWrite(ledPin, HIGH);
    } else if (messageTemp == "off") {
      Serial.println("off");
      //digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void LoRaonReceive(int packetSize) {
  if (packetSize == 0) return;  // if there's no packet, return

  int target = LoRa.read();
  int transmitter = LoRa.read();
  bool state = LoRa.read();

  /*
  String incoming = "";
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
  */

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
  Serial.println((state) ? "TRUE" : "FALSE");
  //Serial.print("RSSI: ");
  //Serial.println(LoRa.packetRssi());
  Serial.println();
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}