
#include <WiFi.h>
#include <esp_now.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <EasyButton.h>
#include <Arduino.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



//Global variables
int beginningTime; //time delay variable
int homeScreenBuffer = 1; 
int varScreenBuffer = 1;
int homeScreenToggle = 1;

int sensorCheck;
int sensorToggle;
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long periodSendRate = 2000;

   //Buttons
#define upPin 19
#define downPin 18
#define enterPin 5

EasyButton buttonUp(upPin);
EasyButton buttonDown(downPin);
EasyButton buttonEnter(enterPin);

//Transmitting portion
uint8_t broadcastAddress[] = {0xA0,0xB7,0X65,0X48,0XC5,0XB0};
String success;
// Must match the device sending structure 
typedef struct var_struct {
int waterLevel;
int ping;
int waterOn;
} var_struct;
var_struct devices;
var_struct robot;

  esp_now_peer_info_t peerInfo;


// Callback function that will be executed when data is sent
void data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
    sensorCheck = 1; //if its low, the display will not show a variable
  }
  else{
    success = "Delivery Fail :(";
    sensorCheck = 0; //if its low, the display will not show the variable
  }
}

// Callback when data is received
void data_receive(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&devices, incomingData, sizeof(devices));
  Serial.print("Bytes received: ");
  Serial.println(len);

}
void CONTROLS(){

/*
home screen scroll wont go past 6, since there are "6 input devices". 
Variable screen scroll wont go past 4 scrolls.
Resets to 1 when max scroll is reached
*/

if(homeScreenToggle)   
  varScreenBuffer = 1;

if(homeScreenBuffer >= 7){
  homeScreenBuffer = 1;
 }
else if(homeScreenBuffer <= 0){
  homeScreenBuffer = 6;
 }

if(varScreenBuffer >= 5){
  varScreenBuffer = 1;
 }
else if(varScreenBuffer <= 0){
  varScreenBuffer = 4;
 }
//END SCROLL BUFFERS

currentMillis = millis();

if(currentMillis - startMillis >= periodSendRate){ //The send rate of the mock robot is 2000ms, this refreshes every 2000ms, will accomodate for delays soon, prolly needs tuned
    robot.ping = !robot.ping;
  //  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &devices, sizeof(devices));
   esp_now_send(broadcastAddress, (uint8_t *) &robot, sizeof(robot));
   startMillis = currentMillis;
}

//debugging
 //Serial.println(homeScreenBuffer);
 //Serial.println(homeScreenToggle);
 //Serial.println(varScreenBuffer);
// Serial.println(sensorCheck);
//end debugging

if(devices.waterLevel >= 3){
    robot.waterOn = 0;  
  //  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &devices, sizeof(devices));
   esp_now_send(broadcastAddress, (uint8_t *) &robot, sizeof(robot));
}

}


void upPressed(){

//when homescreen is active, home screen scroll is active
if(homeScreenToggle)
  homeScreenBuffer += 1;

//when variable screen is active, variable screen scroll is active
if(!homeScreenToggle)
  varScreenBuffer += 1;
}

void downPressed(){

//when homescreen is active, home screen scroll is active
if(homeScreenToggle)
  homeScreenBuffer -= 1;

//when variable screen is active, variable screen scroll is active
if(!homeScreenToggle)
  varScreenBuffer -= 1;
}

void enterPressed(){
//Toggles to variable screen when pressed

if(homeScreenToggle)
  homeScreenToggle = !homeScreenToggle;

//reverts to home screen when scrolled to bottom of variable screen and "Exit" is highlighted
if(!homeScreenToggle && varScreenBuffer == 4)
 homeScreenToggle = !homeScreenToggle;

if(!homeScreenToggle && varScreenBuffer == 2 && homeScreenBuffer == 2){
  if(devices.waterLevel < 3)
    robot.waterOn = !robot.waterOn;
    else if(devices.waterLevel >= 3)
    robot.waterOn = 0;
  //  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &devices, sizeof(devices));
   esp_now_send(broadcastAddress, (uint8_t *) &robot, sizeof(robot));
}
}
 
void blankVarOled(){
  if(varScreenBuffer == 1)
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
 else
  display.setTextColor(SSD1306_WHITE);
  //end highlight
  display.setCursor(15, 21); //sets position 
  display.print("------");   //display device
  display.setCursor(64, 21); //sets position for variable
  display.print("------");


 //Line 2:

 //highlights selected variable
 if(varScreenBuffer == 2)
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
 else
  display.setTextColor(SSD1306_WHITE);
  //end highlight
  display.setCursor(15, 32);  //sets position 
  display.print("------");    //display 
  display.setCursor(64, 32); //sets position for variable
  display.print("------");

 //Line 3:

 //highlights selected variable
 if(varScreenBuffer == 3)
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
 else
  display.setTextColor(SSD1306_WHITE);
  //end highlight
  display.setCursor(15, 43); //sets position 
  display.print("------");   //display 
  display.setCursor(64, 43); //sets position for variable
  display.print("------");


 //Line 4:

 //highlights selected variable
 if(varScreenBuffer == 4)
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
 else
  display.setTextColor(SSD1306_WHITE);
  //end highlight
  display.setCursor(15, 54); //sets position for exit
  display.print("EXIT");     //display EXIT - when pressed will exit to home screen
}

void waterVarOled(){
  if(varScreenBuffer == 1)
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
 else
  display.setTextColor(SSD1306_WHITE);
  //end highlight
  display.setCursor(15, 21); //sets position 
  display.print("Level");   //display device
  display.setCursor(64, 21); //sets position for variable
switch(devices.waterLevel){
case 0:
  display.print("EMPTY");
break;
case 1:
  display.print("LOW");
break;
case 2:
  display.print("MID");
break;
case 3:
 display.print("HIGH");
break;
}
 //Line 2:
 //highlights selected variable
 if(varScreenBuffer == 2)
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
 else
  display.setTextColor(SSD1306_WHITE);
  //end highlight
  display.setCursor(15, 32);  //sets position 
  display.print("Fill?");    //display 
  display.setCursor(64, 32); //sets position for variable
  if(robot.waterOn == 1)
  display.print("filling");
  else 
  display.print("inactive");
 //Line 3:

 //highlights selected variable
 if(varScreenBuffer == 3)
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
 else
  display.setTextColor(SSD1306_WHITE);
  //end highlight
  display.setCursor(15, 43); //sets position 
  display.print("Timer?");   //display 
  display.setCursor(64, 43); //sets position for variable
  display.print("------");


 //Line 4:

 //highlights selected variable
 if(varScreenBuffer == 4)
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
 else
  display.setTextColor(SSD1306_WHITE);
  //end highlight
  display.setCursor(15, 54); //sets position for exit
  display.print("EXIT");     //display EXIT - when pressed will exit to home screen
}

void OLED(){
  if(homeScreenToggle == true){
  display.clearDisplay();                      //clear buffer

  //Setup home screen
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(28, 8);                    //sets position for "controller"
  display.setFont(&FreeSerifItalic9pt7b);      //fancy dancy font
  display.print("Controller");                 //middle top of screen
  display.drawLine(0,16,128,16,SSD1306_WHITE); //line to separate from rest of code 
  display.setFont(NULL);                       //resets font to default 

    //indicating dots on the right side of screen
  display.drawCircle(6,24,3,SSD1306_WHITE); 
  display.drawCircle(6,35,3,SSD1306_WHITE); 
  display.drawCircle(6,46,3,SSD1306_WHITE); 
  display.drawCircle(6,57,3,SSD1306_WHITE); 
  //end homescreen setup


//Device 1:

//highlights selected variable
if(homeScreenBuffer == 1)
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
else
  display.setTextColor(SSD1306_WHITE);
  //end highlight

if(homeScreenBuffer <= 4){   //Only active when selector is under 4 - first page
  display.setCursor(15, 21); //sets position for device
  display.print("Gate");     //display device name - Gate opener
  display.setCursor(64, 21); //sets position for device
  display.print("-");        //blank data, no device connected. "Gate" is only displayed since it is one of our robots
}

//Device 2:

//highlights selected variable
if(homeScreenBuffer == 2)
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
else
  display.setTextColor(SSD1306_WHITE);
  //end highlight

  if(homeScreenBuffer <= 4){  //Only active when selector is under 4 - first page
  display.setCursor(15, 32);  //sets position for device
  display.print("Water");    //display device name - water filler 
  display.setCursor(64, 32); //sets position for device

  if(sensorCheck == 1)       //We have a constant connection with our mock water device 
switch(devices.waterLevel){
case 0:
  display.print("EMPTY");
break;
case 1:
  display.print("LOW");
break;
case 2:
  display.print("MID");
break;
case 3:
 display.print("HIGH");
break;
}
  else if(sensorCheck == 0)   
    display.print("-");           //no device connected. SensorToggle stays the same 
  
  }
//Device 3:

//highlights selected variable
if(homeScreenBuffer == 3)
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
else
  display.setTextColor(SSD1306_WHITE);
  //end highlight

if(homeScreenBuffer <= 4){   //Only active when selector is under 4 - first page
  display.setCursor(15, 43); //sets position for device
  display.print("Feed");   //display device name - feeder
  display.setCursor(64, 43); //sets position for device
  display.print("-");        //blank data, no device connected. "Feed" is only displayed since it is one of our robots
}

//Device 4:

//highlights selected variable
if(homeScreenBuffer == 4)
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
else
  display.setTextColor(SSD1306_WHITE);
  //end highlight

if(homeScreenBuffer <= 4){   //Only active when selector is under 4 - first page
  display.setCursor(15, 54); //sets position for device
  display.print("------");   //display device name 
}

//Device 5:

//highlights selected variable
if(homeScreenBuffer == 5)
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
else
  display.setTextColor(SSD1306_WHITE);
//end highlight

if(homeScreenBuffer > 4){    //Only active when selector is over 4 - second page
  display.setCursor(15, 21); //sets position for device
  display.print("------");   //display device name 
}

//Device 6:

//highlights selected variable
if(homeScreenBuffer == 6)
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);

else
  display.setTextColor(SSD1306_WHITE);  
//end highlight

if(homeScreenBuffer > 4){    //Only active when selector is over 4 - second page
  display.setCursor(15, 32); //sets position for device
  display.print("------");   //display device name 
}
}
//Device 7

//Device 8



//Variable screen
 if(homeScreenToggle == false){
  display.clearDisplay();                      //clear buffer

  //Setup Variable screen
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(28, 8);                    //sets position for respective variable
  display.setFont(&FreeSerifItalic9pt7b);      //fancy dancy font
  
  //displays what goes on top of robots screen
  switch (homeScreenBuffer){

  case 1:
  display.print("Gate"); //Fake  
  break;

  case 2:
  display.print("Water level");  //Mock robot 
  break;

  case 3:
  display.print("Feed level"); //Fake
  break;

  case 4:
  homeScreenToggle = !homeScreenToggle; //exits unused slot
  break;
  case 5:
  homeScreenToggle = !homeScreenToggle; //exits unused slot
  break;
  case 6:
  homeScreenToggle = !homeScreenToggle; //exits unused slot
  break;
  }

  display.drawLine(0,16,128,16,SSD1306_WHITE); //line to separate from rest of code 
  display.setFont(NULL);                       //resets font to default 

  //indicating dots on the right side of screen
  display.drawCircle(6,24,3,SSD1306_WHITE); 
  display.drawCircle(6,35,3,SSD1306_WHITE); 
  display.drawCircle(6,46,3,SSD1306_WHITE); 
  display.drawCircle(6,57,3,SSD1306_WHITE); 
  //end variable screen setup

// Functions for each robot. Notice most are blankVarOled and we have a custom one for our "Mock" robot. 
switch (homeScreenBuffer){
 case 1:
 blankVarOled();
 break;
 case 2:
 waterVarOled();;
 break;
 case 3:
 blankVarOled();
 break;
 case 4:
 blankVarOled();
 break;
 case 5:
 blankVarOled();
 break;
 case 6:
 blankVarOled();
 break;
}

}
//End var Screen Set Up

display.display(); //displays information. 


}


void setup() {
//MCU INIT
  Serial.begin(9600); // not anything higher, run into issues with displaying data on serial.monitor
//END MCU SETUP
//pinMode(buzzer, OUTPUT); //deleted. over current on pin. Lowest resistor size available wont make a tone. 

//ESP_NOW INIT
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) { //debugging to make sure shit it working. 
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
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(data_receive);

//END ESP_NOW



//DISPLAY INIT
   // SSD1306_SWITCHCAPVCC = generate display voltage from buffered 3.3v output
 display.setTextColor(SSD1306_WHITE); //needs to be set as white initially. 
 display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS); //Screen address is 0x3C even though it should be 0x3D? IDK this works.
   // Clear the buffer
 display.clearDisplay();
//END DISPLAY

//BUTTON INIT
  buttonUp.begin();
  buttonDown.begin();
  buttonEnter.begin();
  //Debounce times. 
  buttonUp.onPressed(upPressed); 
  buttonDown.onPressed(downPressed);
  buttonEnter.onPressed(enterPressed);
//END BUTTON
}



void loop() {
//beginningTime = millis(); //start time delay 
CONTROLS(); 
OLED();
buttonUp.read();
buttonDown.read();
buttonEnter.read();
//Serial.println(millis()-beginningTime); //print time delay. 26ms from start to finish 18Sep2023
}
