/*
 * KRVS - Remote control interface for Denon BN-500BD MKII CD Player
 * Converts dry contact to TCP commands for 3 different CD players (Play / Pause)
 * July 2022 - KW (Some code recycled from EAS switch project)
 * Based on OLIMEX ESP32-POE and the SSD1306 for status display
 * The program started to change into a single remote for each player (was going to move from ethernet
 * to serial) because the player would not do things like track up and pause.  A code work around was
 * found and started to be implemented.  Next it was learned that track remain time could
 * not be derived from the remote commands.  This put the project on infinate hold.
 * 
 * A code review would be appreciated though for future projects.
 * 7/19/22 kw
 */

//ETH Hardware Stuff
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT 
#include <ETH.h>

// SSD1306 I2C Setup --- PINS 13 and 16 RESERVED on OLIMEX32-POE
#include <SSD1306Wire.h> // legacy include: `#include "SSD1306.h"
SSD1306Wire display(0x3c, 13, 16, GEOMETRY_128_64 );  // (Address, SDA, SCL, Geometry) for SSD1306 OLED on ESP32-POE

WiFiClient client;

// network variables
IPAddress deviceOneIp(192, 168, 3, 91); //IP address for CD Player 1.  Must use commas!
IPAddress deviceTwoIp(192, 168, 3, 92); //IP address for CD Player 2.  Must use commas!
IPAddress deviceThreeIp(192, 168, 3, 93); //IP address for CD Player 3.  Must use commas!
//IPAddress deviceCompIp(192, 168, 3, 97); //IP address for Test Computer (Debug).  Must use commas!

const unsigned int deviceOneTcp = 9030; //tcp port to connect to
const unsigned int deviceTwoTcp = 9030; //tcp port to connect to
const unsigned int deviceThreeTcp = 9030; //tcp port to connect to

bool ethConnected = false;

bool deviceOneLast = false;
bool deviceTwoLast = false;
bool deviceThreeLast = false;

// CD Command Variables
String devicePlay = String("@02353\r"); // PLAY command
String devicePause = String("@02348\r"); // PAUSE command

// Variables for the Inturrupts
struct buttonOne { // create Button struct (probably overkill but may need more variables)
  bool pressed;
};
buttonOne button1 = {true}; // set initial state

struct buttonTwo { // button for track up
  bool pressed;
};
buttonTwo button2 = {false}; // set initial state


void IRAM_ATTR buttonOne() { // -------- Setup Inturrupt Function for state -----------------
  button1.pressed = digitalRead(14);
  }

void IRAM_ATTR buttonTwo() { // --------- Setup Inturrupt Function for track --------------
  button2.pressed = digitalRead(15);
}

void WiFiEvent(WiFiEvent_t event) { // -------------- Network Voodoo -------------------
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

void playerOne() { // ----------- function for Player One play/pause -----------------------

  if (button1.pressed == LOW) {

    WiFiClient client;
    if (!client.connect(deviceOneIp, deviceOneTcp)) {
    Serial.println("connection failed");
    return;
    }
    //client.connect(IPAddress(deviceOneIp), deviceOneTcp);
    //client.print(String(devicePlay)); // does not work no ack
    client.printf("@02353\r", deviceOneIp); // why this syntax?
    String response = client.readStringUntil('\n');
    Serial.println(response);
    client.stop();

    display.clear();
    display.drawString(1, 20, String("CD 1 Play   "));
    Serial.println(devicePlay);
    display.display();
    deviceOneLast = LOW;
    
    } else  {

    WiFiClient client;
    if (!client.connect(deviceOneIp, deviceOneTcp)) {
    Serial.println("connection failed");
    return;
    }
    //client.connect(IPAddress(deviceOneIp), deviceOneTcp);
    //client.print(String(devicePause)); // does not work no ack
    client.printf("@02348\r", deviceOneIp); // why this syntax?
    String response = client.readStringUntil('\n');
    Serial.println(response);
    client.stop();

    display.clear();
    display.drawString(1, 20, String("CD 1 Pause"));
    Serial.println(devicePause);
    display.display();
    deviceOneLast = HIGH;
  }
   
}  
void playerOneTrackUp() { // ----------- function to track Up and Pause -----------------

    WiFiClient client;
    if (!client.connect(deviceOneIp, deviceOneTcp)) {
    Serial.println("connection failed");
    return;
    }
    
    client.printf("@02332\r", deviceOneIp); // Tack Number
    
    String response = client.readStringUntil('\n');
    Serial.print(response);
    Serial.println("  Track Up");
    delay(10);
    client.printf("@02348\r", deviceOneIp); // Pause
    response = client.readStringUntil('\n');
    Serial.print(response);
    Serial.println("  Pause");
    client.stop();

    display.clear();
    display.drawString(1, 40, String("Tk UP"));
    display.display();
    button2.pressed = false;
        
}


void playerOneTrack() { // ----------- function to get track # ---------------------------

    WiFiClient client;
    if (!client.connect(deviceOneIp, deviceOneTcp)) {
    Serial.println("connection failed");
    return;
    }
    
    client.printf("@0?PCGp\r", deviceOneIp); // Tack Number
    String response = client.readStringUntil('\n');
    Serial.print(response);
    Serial.println(  "track number");
    client.stop();

    String track =  response.substring(12,15);

    display.clear();
    display.drawString(1, 30, String("Tk: "));
    display.drawString(25, 30, track);
    display.display();
    
}  
// -------------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200); // start serial monitor

// --- Init the SSD1306 OLED ---
  pinMode(16, OUTPUT); // little code dance to reset the SSD1306 OLED via hardware
  digitalWrite(16, LOW);
  delay(50);
  digitalWrite(16, HIGH);
  display.init();
  display.flipScreenVertically();

  display.clear();
  display.setFont(ArialMT_Plain_16); // Options 10pt,16pt,24pt
  display.drawString(1, 1, String("Standby"));
  display.display();
  delay(2000); // delay for display
  
// Init ESP32 eth with a local address:
  IPAddress ip(192,168,3,95);
  IPAddress gateway(192,168,3,1);
  IPAddress subnet(255,255,255,0);
  IPAddress dns(4,4,4,4);
  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  ETH.config(ip, gateway, subnet, dns);

  delay(5000); // wait for ETH to get online

// ISR
  pinMode(14, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(14), buttonOne, CHANGE);
  pinMode(15, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(15), buttonTwo, FALLING);

// Some status info to the OLED
  display.clear();
  display.drawString(1, 1, String("Ready to go..."));
  display.display();
  delay(2000); // delay for display
  display.clear();

  client.connect(IPAddress(deviceOneIp), deviceOneTcp); // Lets see if this works

} // end of setup

void loop() {
 
    display.setFont(ArialMT_Plain_16); // Options 10pt,16pt, 24pt
    display.drawString(1, 1, String("Status V2.7:")); // cause why not?
    display.drawString(90, 1, String(button1.pressed)); //debug
    display.drawString(110, 1, String(deviceOneLast));  //debug
    display.display();
 
// check if the pushbutton is equal to last known state
  if (deviceOneLast != button1.pressed) {
    playerOne();
       
  } else if (button2.pressed == true){
      playerOneTrackUp();
    
  } else {
    playerOneTrack();
    // do nothing... just keep looping waiting for a state change
  }

/*
Serial.print(deviceOneLast); //debug
Serial.print(button1.pressed); //debug
Serial.println(); //debug
*/

delay(1000); //sanity pause

} // end of loop
