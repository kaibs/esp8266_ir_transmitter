#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>
#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

//ir led
const uint16_t IR_LED_PIN = 0; //GPIO5 on ESP01
IRsend irsend(IR_LED_PIN);

//include passwords
#include "credentials.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER_IP;
const char* user= MQTT_USER;
const char* passw= MQTT_PASSWORD;

//Wifi
WiFiClient EspIR;
PubSubClient client(EspIR);
String receivedString;

//send cmnd to tv
void transmitTV(String cmnd){
  
  if(cmnd == "TOGGLE"){
    irsend.sendRC6(0xC, 20);
    Serial.println("TV ON/OFF");
  }

  if(cmnd == "MUTE"){
    irsend.sendRC6(0x1000D, 20);
    Serial.println("TV MUTE");
  }
}

//send cmnd to hifi
void transmitHIFI(String cmnd){

  if(cmnd == "VOL_UP"){
    irsend.sendNEC(0xA156F906, 32);
    Serial.println("Hifi VOL_UP");
  }

  if(cmnd == "VOL_DOWN"){
    irsend.sendNEC(0xA1567986, 32);
    Serial.println("Hifi VOL_DOWN");
  }

  if(cmnd == "TOGGLE"){
    irsend.sendNEC(0xA156E916, 32);
    Serial.println("Hifi ON/OFF");
  }

  if(cmnd == "MUTE"){
    irsend.sendNEC(0xA1569966, 32);
    Serial.println("Hifi MUTE");
  }

  if(cmnd == "AUX2"){
    irsend.sendNEC(0xA15624DB, 32);
    Serial.println("Hifi AUX2");
  }

  if(cmnd == "AUX3"){
    irsend.sendNEC(0xA15628D7, 32);
    Serial.println("Hifi AUX3");
  }
}

//callback for receiving MQTT
void callback(char* topic, byte* payload, unsigned int length) {

 if (strcmp(topic,"home/bedroom/ir_transmitter/tv")==0){
 
  for (int i=0;i<length;i++) {
   receivedString += (char)payload[i];
  }
  transmitTV(receivedString);
  receivedString = "";
 }

 if (strcmp(topic,"home/bedroom/ir_transmitter/hifi")==0){
 
  for (int i=0;i<length;i++) {
   receivedString += (char)payload[i];
  }
  transmitHIFI(receivedString);
  receivedString = "";
 }
}

//reconnect for mqtt-broker
void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect
 if (client.connect("EspIR", user, passw)) {
  Serial.println("connected");

  //subscribe
  client.subscribe("home/bedroom/ir_transmitter/tv");
  client.subscribe("home/bedroom/ir_transmitter/hifi");
  
 } else {
  Serial.print("failed, rc=");
  Serial.print(client.state());
  Serial.println(" try again in 5 seconds");
  // Wait 5 seconds before retrying
  delay(5000);
  }
 }
}

void setup() {
  
  Serial.begin(9600);
  irsend.begin();

  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50); 

  //Wifi
  WiFi.hostname("EspIR");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  //mqtt
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void loop() {

  //check mqtt connection
  if (!client.connected()){
    reconnect();
  }
  client.loop();

}