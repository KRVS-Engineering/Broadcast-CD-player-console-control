/*
 * KRVS - Remote control interface for Denon BN-500BD MKII CD Player
 * Converts dry contact to TCP commands for 3 different CD players (Play / Pause)
 * July 2022 - KW (Some code recycled from EAS switch project)
 * Based on OLIMEX ESP32-POE and the SSD1306 for status display
 */
 
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT //ETH stuff?
#include <ETH.h>

#include <SSD1306Wire.h> // legacy include: `#include "SSD1306.h"

// SSD1306 Info
SSD1306Wire display(0x3c, 13, 16, GEOMETRY_128_64 );  // (Address, SDA, SCL, Geometry) for SSD1306 OLED on ESP32-POE

// setup digital I/O (will need 3 Inputs eventually)
//#define buttonPin1 14     // dry contact input #2
//#define buttonPin2 15    // dry contact input #2
//#define buttonPin3 2    // dry contact input #3
// bool buttonState = false;    // variable for reading the pushbutton status


WiFiClient client;



// network vars (these are left over from some old code I recycled.  Need to be CLEANED UP)
IPAddress deviceOneIp(192, 168, 3, 91); //IP address for CD Player 1.  Must use commas!
//IPAddress deviceTwoIp(192, 168, 3, 92); //IP address for CD Player 2.  Must use commas!
//IPAddress deviceThreeIp(192, 168, 3, 93); //IP address for CD Player 3.  Must use commas!

const unsigned int deviceOneTcp = 9030; //tcp port to connect to
const unsigned int deviceTwoTcp = 9030; //tcp port to connect to
const unsigned int deviceThreeTcp = 9030; //tcp port to connect to

bool ethConnected = false;

bool deviceOneState = false;
bool deviceTwoState = false;
bool deviceThreeState = false;

bool deviceOneLast = false;
bool deviceTwoLast = false;
bool deviceThreeLast = false;

// CD Command Variables
String devicePlay = String("@02353\r"); // open command
String devicePause = String("@02348\r"); // close command

struct Button { // create Button struct
  bool pressed;
};

Button button1 = {false}; // set initial state

void IRAM_ATTR isr_low() { // Button is pressed
  button1.pressed = digitalRead(14);

  }


void WiFiEvent(WiFiEvent_t event) { // Network Voodoo
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      ETH.setHostname("CD-Player-Control");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      ethConnected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      ethConnected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      ethConnected = false;
      break;    
  }
}

  

void playerOne() {

  if (button1.pressed == LOW) {
    client.connect(IPAddress(deviceOneIp), deviceOneTcp);
    client.print(String(devicePlay));
    //delay(100);
    client.stop();
    display.clear();
    display.drawString(1, 30, String("CD 1 Play   "));
    Serial.println("CD 1 Play");
    display.display();
    deviceOneLast = LOW;
          
    } else  {
      
    client.connect(IPAddress(deviceOneIp), deviceOneTcp);
    //delay(100);
    client.print(String(devicePause));
    client.stop();
    display.clear();
    display.drawString(1, 30, String("CD 1 Pause"));
    Serial.println("CD 1 Pause");
    display.display();
    deviceOneLast = HIGH;
}
  } 
  


void setup() {
  Serial.begin(115200); // start serial monitor

  // Init ESP32 eth with a local address:
  IPAddress ip(192,168,3,95);
  IPAddress gateway(192,168,3,1);
  IPAddress subnet(255,255,255,0);
  IPAddress dns(4,4,4,4);
  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  ETH.config(ip, gateway, subnet, dns);



  // --- Init the SSD1306 OLED ---
  pinMode(16, OUTPUT); // little code dance to reset the SSD1306 OLED via hardware
  digitalWrite(16, LOW);
  delay(50);
  digitalWrite(16, HIGH);

  display.init();
  display.flipScreenVertically();

    display.clear();
    display.setFont(ArialMT_Plain_16); // Options 10pt,16pt, 24pt
    display.drawString(1, 1, String("ETH READY"));
    display.display();
    delay(1000);
    display.clear();

  //pinMode(buttonPin1, INPUT_PULLUP); // config GPIO as input pin and enable the internal pull-up resistor
  //pinMode(buttonPin2, INPUT_PULLUP); // config GPIO as input pin and enable the internal pull-up resistor
  //pinMode(buttonPin3, INPUT_PULLUP); // config GPIO as input pin and enable the internal pull-up resistor

 // ISR
 pinMode(14, INPUT_PULLUP);
 attachInterrupt(digitalPinToInterrupt(14), isr_low, CHANGE);
 

} // end of setup

void loop() {
 
    display.setFont(ArialMT_Plain_16); // Options 10pt,16pt
    display.drawString(1, 1, String("Status V3:"));
    display.drawString(80, 1, String(button1.pressed));
    display.drawString(100, 1, String(deviceOneLast));
    display.display();
 
// check if the pushbutton is equal to last known state
  if (deviceOneLast != button1.pressed) {
    playerOne();
      
  } 
  else {
    
  }
     
  

Serial.print(deviceOneLast); //debug
Serial.print(button1.pressed); //debugf

delay(1000);

   
} // end of loop
