#include "Utilities.h"
#include "Light.h"
#include "Button.h"
#include <Adafruit_NeoPixel.h>

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
Adafruit_MQTT_Publish statusFeed = Adafruit_MQTT_Publish(&mqtt, "steyaertHome/masterBedroom/lightsStatus");
Adafruit_MQTT_Subscribe controlFeed = Adafruit_MQTT_Subscribe(&mqtt, "steyaertHome/masterBedroom/lightsControl");


// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
// Encoder myEnc(5, 6);
//long oldEncoderPosition  = -999;


// pinouts availbale 0 2 4 5 12 13 14 15 16
#define NEOPIXEL_PIN   12
#define NUMPIXELS      59
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// int timerTicks = 0;               // count ticks for interrupt timer

// create objects
#define LIGHT_COUNT 3
Light light[LIGHT_COUNT];
#define BUTTON_COUNT 2
Button button[BUTTON_COUNT];
Utilities utilities;

// TIME
// NTP Servers:
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov

const int timeZone = -5;     // Central Daylight Time
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
//time_t prevDisplay = 0; // when the digital clock was displayed


void setup() {
  utilities.setup();
  
  controlFeed.setCallback(controlCallback);
  
  // Setup MQTT subscription for time feed.
  mqtt.subscribe(&controlFeed);
  MQTT_connect();

  controlFeed.setCallback(controlCallback);
  
  // Setup MQTT subscription
  mqtt.subscribe(&controlFeed);

  // set up lights
  light[0].setup( 0, 19, 0, "01", setPixelColor, MQTT_publish);
  light[1].setup(20, 38, 1, "02", setPixelColor, MQTT_publish);
  light[2].setup(39, 58, 2, "03", setPixelColor, MQTT_publish);

  // set up pushbuttons
  button[0].setup(4, 0, buttonPress);
  button[1].setup(5, 2, buttonPress);

  // rotaryEncoder.setup();

  pixels.begin();  // This initializes the NeoPixel library.
  pixels.show();   // Initialize all pixels to 'off'

// TIME

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
}


void buttonPress(bool value) {
  if (value) {
    setPixelColor(1, 200, 200, 200);
    Serial.println("on");
  } else {
    setPixelColor(1, 0, 0, 0);
    Serial.println("off");
  }
  pixels.show();
}


void setPixelColor (int number, int red, int green, int blue) {
  pixels.setPixelColor(number, pixels.Color(red, green, blue));
}


void loop() {
  // Ensure the connection to the MQTT server stays alive
  MQTT_connect();

  // check MQTT feed for updates
  mqtt.processPackets(20);

//  utilities.update();
<<<<<<< HEAD
  updateInterrupts();     // allow interrupt timer to run

  delay(20);  // try to save battery but keep responsiveness? *** TODO: Tweak this to maximum delay while keeping responsiveness
=======
  // updateInterrupts();     // allow interrupt timer to run

  // update pushbuttons
  for (int buttonID = 0; buttonID < BUTTON_COUNT; buttonID++) {
    button[buttonID].update();
  }

//  long newPosition = myEnc.read();
//  if (newPosition != oldEncoderPosition) {
//    oldEncoderPosition = newPosition;
//    Serial.println(newPosition);
//  }
>>>>>>> parent of 2234290... Better dimming, clean-up, added time
}


void controlCallback(char *data, uint16_t len) {
  Serial.print("Received: ");
  Serial.println(data);
  for (int i = 0; i < LIGHT_COUNT; i++) {
    light[i].processMessage(data);
  }
}


<<<<<<< HEAD
void updateInterrupts() { 
  if (light[2].value > -1) {  // temp override
    // if third light at 1%, show time
    if (timerTicks++ > 1001) {
      // new NTP request every ~30 seconds
      Serial.println("update");
      light[2].updateTime();
      utilities.loop();
      timerTicks = 0;
    }
  } else if ((startingUp) || (light[1].value == 4)) {
    // show startup sequence
    runSequence("rainbow");
    startingUp = false;
  }
}


void runSequence(char *sequenceName) {
  if (sequenceName == "rainbow") {
    for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
      for (int q=0; q < 3; q++) {
        light[2].theaterChaseRainbow(j, q);
        pixels.show();
        delay(5);
      }
    }
  } else if (sequenceName == "morning") {
    light[2].hue = 2; // red
    light[2].saturation = 100;
    light[2].value = 0;
    // now set this value
    // *** TODO: get the transition to delay slowly: Create a transition method?
    // end point
    light[2].hue = 100; // blue?
    light[2].saturation = 100;
    light[2].value = 100;
=======
/* if we need to add multi-threading later...
void updateInterrupts() {
  if (timerTicks++ > 2001) {
    timerTicks = 0;
    timerTicks = 0;
  } else if (timerTicks % 100 == 0) {
    light[0].update();
    light[1].update();
>>>>>>> parent of 2234290... Better dimming, clean-up, added time
  }
}
*/


void MQTT_publish (String message) {
  // light status format: ID:{ON/OFF}:HUE:SAT:VAL
  char* charArray = "00:XXX:000:000:000";
  message.toCharArray(charArray, message.length()+1);
  statusFeed.publish(charArray);
  Serial.print("Published:");
  Serial.println(message);
  pixels.show();
}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
     Serial.println(mqtt.connectErrorString(ret));
     Serial.println("Retrying MQTT connection in 5 seconds...");
     mqtt.disconnect();
     delay(5000);  // wait 5 seconds
     retries--;
     if (retries == 0) {
       // basically die and wait for WDT to reset me
       while (1);
     }
  }
  Serial.println("MQTT Connected!");
}


/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime() {
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
