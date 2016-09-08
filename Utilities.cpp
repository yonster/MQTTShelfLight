/*************************   Various Setup   *********************************/
#include "Utilities.h"

Utilities::Utilities() {
}


void Utilities::setup() {
  // Subscribe to these from Terminal with e.g.:
  // mosquitto_sub -h 192.168.1.65 -d -t steyaertHome/masterBedroom/lightsStatus

  Serial.begin(115200);
  delay(5);
  
  // Connect to WiFi access point.
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
}


//void Utilities::update() {
//}
//
//
//void Utilities::addMQTTObject(Object object) {
//  objects[objectCount++] = object;
////  object.setCallback(MQTT_publish);
////  bedroomLightsControlFeed.setCallback(object.tempCallback);
//  Serial.println("added");
//}

void Utilities::loop() {
//  if (timeStatus() != timeNotSet) {
//    if (now() != prevDisplay) { //update the display only if time has changed
//      prevDisplay = now();
      digitalClockDisplay();  
//    }
//  }
}


//float Utilities::getTime() {
//  // returns time as HH.MMM (minutes as decimal)
//  return hour() + (minute() / 60);
//}

void Utilities::digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
//  printDigits(second());
//  Serial.print(" ");
//  Serial.print(day());
//  Serial.print(".");
//  Serial.print(month());
//  Serial.print(".");
//  Serial.print(year()); 
  Serial.println(); 
}


void Utilities::printDigits(int digits) {
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
