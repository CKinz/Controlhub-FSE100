/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-one-to-many-esp8266-nodemcu/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/


//MOCK ROBOT for FSE100


#include <WiFi.h>
#include <esp_now.h>

// REPLACE WITH RECEIVER MAC Address - WE ARE USING BOARD 1 AS RECIEVER
uint8_t broadcastAddress[] = {0xA0, 0xB7, 0x65, 0x4F, 0x09, 0xB4};
//uint8_t broadcastAddress2[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
int toggler;

// Structure example to send data
// Must match the receiver structure

String success;


typedef struct var_struct {
int waterLevel;
int ping;
int waterOn;
} var_struct;

// Create a struct_message called myData
var_struct devices;
var_struct robot;
 esp_now_peer_info_t peerInfo;

// callback when data is sent
void data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}
int led = 33;
int highLevel = 32;
int midLevel = 35;
int lowLevel = 34;
// Callback when data is received
void data_receive(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&robot, incomingData, sizeof(robot));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.println(robot.waterOn);
}

void setup() {
  // Init Serial Monitor
  Serial.begin(9600);
 pinMode(led, OUTPUT);
  pinMode(lowLevel, INPUT);
   pinMode(midLevel, INPUT);
    pinMode(highLevel, INPUT);
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

esp_now_register_send_cb(data_sent);
  
 // esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;       
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(data_receive);
}
 
void loop() {
  digitalWrite(14, HIGH);
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &devices, sizeof(devices));
if(robot.waterOn == 1)
  digitalWrite(led, HIGH);
  else
  digitalWrite(led, LOW);
  // Send message via ESP-NOW
if(digitalRead(highLevel) == HIGH)
  devices.waterLevel = 3;
else if(digitalRead(midLevel) == HIGH)
  devices.waterLevel = 2;
else if(digitalRead(lowLevel) == HIGH)
  devices.waterLevel = 1;
else devices.waterLevel = 0;
Serial.println(devices.waterLevel);

    esp_now_send(broadcastAddress, (uint8_t *) &devices, sizeof(&devices));
  delay(2000);


}