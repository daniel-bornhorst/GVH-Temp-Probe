#define DEBUG 1

#ifdef DEBUG
# define DEBUG_PRINTLN(x) Serial.println(x);
# define DEBUG_PRINT(x) Serial.print(x);
#else
# define DEBUG_PRINTLN(x)
# define DEBUG_PRINT(x)
#endif

/*
 *  This sketch sends random data over UDP on a ESP32 device
 *
 */
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include "elapsedMillis.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi network name and password
// const char* networkName = "MrBrightSign";
// const char* networkPswd = "GrabFinkleStern3967%";

const char* networkName = "MW-IOT";
const char* networkPswd = "CrimpleLump52#";

//IP address to send UDP data to:
// either use the ip address of the server or 
// a network broadcast address
const char * udpAddress = "10.32.70.24";
const int udpPort = 9000;

const unsigned int localPort = 6667;

// const char * udpAddress = "10.32.16.123";
// const int udpPort = 6543;

//Are we currently connected?
boolean connected = false;

//The udp library class
WiFiUDP Udp;

const int led = LED_BUILTIN;


// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS A0

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


elapsedMillis repeatTriggerTimer;
long unsigned int repeatTriggerTime = 15000; // 15 seconds


OSCErrorCode error;


void setup()
{
  // Initilize hardware serial:
  Serial.begin(115200);
  delay(10);

  sensors.begin();

  delay(10000);
  
  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);

  Udp.begin(localPort);

  repeatTriggerTimer = repeatTriggerTime; // allow for button push immediately after boot
}

void loop() {

  checkForOSCMessage();

  // sensors.requestTemperatures(); // Send the command to get temperatures
  // float tempF = sensors.getTempFByIndex(0);

  // // Check if reading was successful
  // if(tempC == DEVICE_DISCONNECTED_C) {
  //   DEBUG_PRINTLN("Error: Could not read temperature data");
  // }

  // DEBUG_PRINT("Temperature for DS1820 is: ");
  // DEBUG_PRINTLN(tempF);

  delay(10);
}


void checkForOSCMessage() {
  OSCMessage msg;
  int size = Udp.parsePacket();

  if (size > 0) {
    DEBUG_PRINTLN("OSC Received!");
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {
      msg.dispatch("/read_temp", echoTemp);
    } else {
      error = msg.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
}


void echoTemp(OSCMessage &msg) {

  sensors.requestTemperatures(); // Send the command to get temperatures
  float tempF = sensors.getTempFByIndex(0);

  // Check if reading was successful
  if(tempF == DEVICE_DISCONNECTED_C) {
    DEBUG_PRINTLN("Error: Could not read temperature data");
    return;
  }

  DEBUG_PRINT("Temperature for DS1820 is: ");
  DEBUG_PRINTLN(tempF);

  Udp.beginPacket(Udp.remoteIP(), udpPort);
  msg.add(tempF);
  msg.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message
}


void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
          //When connected set 
          DEBUG_PRINT("WiFi connected! IP address: ");
          DEBUG_PRINTLN(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          Udp.begin(WiFi.localIP(),localPort);
          connected = true;
          break;
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
          DEBUG_PRINTLN("WiFi lost connection");
          connected = false;
          break;
      default: break;
    }
}

