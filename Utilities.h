/*
  Utilities.h - setup utilities
*/

#ifndef Utilities_h
#define Utilities_h
#include "Arduino.h"
#include "Object.h"


/************************* Time *********************************/

#include <SPI.h>
#include <Wire.h>
#include <TimeLib.h> 

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "ATTuNvpygs"
#define WLAN_PASS       "f4hee83p6qir"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

/************************* MQTT Setup *********************************/

#define MQTT_SERVER      "192.168.1.65"
#define MQTT_PORT   1883        // use 8883 for SSL
#define MQTT_USERNAME    ""
#define MQTT_PASSWORD    ""
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#ifdef __AVR__
  #include <avr/power.h>
#endif

class Utilities {
  public:
    Utilities();
    void setup();
//    void MQTT_connect();
//    void MQTT_publish(String message);
//    void update();
//    void addMQTTObject(Object object);
    void timeUpdate();
  private:
    unsigned int localPort = 2390;      // local port to listen for UDP packets
    IPAddress timeServerIP; // time.nist.gov NTP server address
    const char* ntpServerName = "time.nist.gov";
    void digitalClockDisplay();
    void printDigits(int digits);
    
    // A UDP instance to let us send and receive packets over UDP
    WiFiUDP udp;
};

#endif
