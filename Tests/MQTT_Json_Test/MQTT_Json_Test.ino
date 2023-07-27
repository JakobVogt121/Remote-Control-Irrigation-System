#include <WiFi.h>
#include "PubSubClient.h"
#include <Wire.h>
const char* ssid = "Vodafone-93E6";
const char* password = "JFtEaFTgbhcpCRUG";
// Add your MQTT Broker IP address, example:
const char* mqtt_server = "192.168.0.236";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

#include <ArduinoJson.h>
StaticJsonDocument<200> doc;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  delay(1000);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  doc["Sender"] = "Valve1";
  doc["Empfänger"] = 1351824120;
  doc["State"] = true;
  delay(1000);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  //digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  //String messageTemp;

  Serial.println((message) ? "TRUE" : "FALSE");

  /*for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();*/

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  //if (String(topic) == "com/nodered") {
  //  Serial.print("Changing output to ");
    if (message){ //messageTemp == "on") {
      Serial.println("on");
      digitalWrite(LED_BUILTIN, HIGH);
    } else{// if (messageTemp == "off") {
      Serial.println("off");
      digitalWrite(LED_BUILTIN, LOW);
    }
  //}
}

void reconnect() {
  // Loop until we're reconnected
  Serial.println("reconnecting");
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("com/nodered");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  //Serial.print("hallo\n");
  //Serial.println("loope");
  //delay(2000);
  client.loop();
  //Serial.println((client.loop()) ? "TRUE" : "FALSE");
  delay(2000);

  //long now = millis();
  /*if (now - lastMsg > 5000) {
    lastMsg = now;

    char buffer[256];
    serializeJson(doc, buffer);
    //  serializeJsonPretty(doc, Serial);
    client.publish("esp32/temperature", buffer);

    //Serial.print("Temperature: ");
    //Serial.println("hallo");
    //client.publish("esp32/temperature", "hallo");
  }*/
}