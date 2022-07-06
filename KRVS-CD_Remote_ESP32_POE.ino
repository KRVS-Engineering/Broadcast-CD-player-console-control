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
#define buttonPin 14     // the number of the pushbutton pin
int buttonState = 0;         // variable for reading the pushbutton status

WiFiClient client;

// network vars (these are left over from some old code I recycled.  Need to be CLEANED UP)
IPAddress switchTwoIp(192, 168, 3, 91); //IP address for CD Player 1.  Must use commas!
// IPAddress switchThreeIp(10, 111, 27, 27); //IP address for switch 3.  Must use commas!
const unsigned int tcpPort = 9030; //tcp port to connect to
bool ethConnected = false;

// CD Command Variables
String stringOne = String("@0PCDTRYOP\r"); // open command
String stringTwo = String("@0PCDTRYCL\r"); // close command


void WiFiEvent(WiFiEvent_t event) { // Network Voodoo
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      ETH.setHostname("esp32-ethernet");
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
  // display.flipScreenVertically();

  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT_PULLUP); // config GPIO as input pin and enable the internal pull-up resistor
  
}

void loop() {
    
    
    display.clear();
    display.setFont(ArialMT_Plain_16); // Options 10pt,16pt
    display.drawString(1, 1, String("Status:"));
    display.display();
    
    
// read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  
// check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == LOW) {
    
    client.connect(IPAddress(switchTwoIp), tcpPort);
    client.print(String(stringOne));
    client.stop();
    display.drawString(1, 30, String("Door Open   "));
    Serial.println(" Door Opened ");
    display.display();

  } else {

    client.connect(IPAddress(switchTwoIp), tcpPort);
    client.print(String(stringTwo));
    client.stop();
    display.drawString(1, 30, String("Door Closed"));
    Serial.println(" Door Closed ");
    display.display();
    
  }
    
delay(400);

   
} // end of loop
