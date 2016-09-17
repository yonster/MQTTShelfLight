#include "Utilities.h"
#include "Light.h"
#include <Adafruit_NeoPixel.h>


WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
Adafruit_MQTT_Publish statusFeed = Adafruit_MQTT_Publish(&mqtt, "steyaertHome/masterBedroom/lightsStatus");
Adafruit_MQTT_Subscribe controlFeed = Adafruit_MQTT_Subscribe(&mqtt, "steyaertHome/masterBedroom/lightsControl");


// pinouts availbale 0 2 4 5 12 13 14 15 16
#define NEOPIXEL_PIN   12
#define NUMPIXELS      59
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

long timerTicks = 0;               // count ticks for interrupt timer
bool rainbowMode = true;           // initiates rainbow startup sequence

// create objects
#define LIGHT_COUNT 3
Light light[LIGHT_COUNT];
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

  pixels.begin();  // This initializes the NeoPixel library.
  pixels.show();   // Initialize all pixels to 'off'

// TIME

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  Serial.println("setup complete");
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
  updateInterrupts();     // allow interrupt timer to run
//  Serial.println("updated interrupts");

  delay(20);  // try to save battery but keep responsiveness? *** TODO: Tweak this to maximum delay while keeping responsiveness
}


void controlCallback(char *data, uint16_t len) {
  Serial.print("Received: ");
  Serial.println(data);

  String messageString = String(data);
  int currentLightID = -1;

  for (int i = 0; i < LIGHT_COUNT; i++) {
    if (strncmp(data, light[i].lightIDString, 2) == 0) {   // compare first 2 chars to lightID, see if it matches
      currentLightID = i;
    }
  }

  if (messageString.substring(3,8) == "CLOCK") {
    light[currentLightID].clockMode = true;
  } else if (messageString.substring(0,7) == "RAINBOW") {
    rainbowMode = true;
  } else if (messageString.substring(3,5) == "ON") {
    light[currentLightID].status = "ON";
  } else if (messageString.substring(3,6) == "OFF") {
    light[currentLightID].status = "OFF";
  } else if (messageString.substring(3,9) == "CHECKSTATUS") {
    // null command to get a status returned
  } else if (messageString.substring(3,11) == "IDENTIFY") {
    // identify light by blinking green a few times
    light[currentLightID].setColor(0, 255, 0);
    for (int j=0; j<8; j++) {
      light[currentLightID].toggle();
      delay(500);
    }
  } else {
    int _value = atoi(&data[7]);
    if (messageString.substring(3,6) == "HUE") {
      light[currentLightID].hue = _value;
    } else if (messageString.substring(3,6) == "SAT") {
      light[currentLightID].saturation = _value * 255/100;
    } else if (messageString.substring(3,6) == "VAL") {
      light[currentLightID].value = _value * 255/100;
    } else if (messageString.substring(3,6) == "DIM") {
      light[currentLightID].value -= 1;
      if (light[currentLightID].value < 0) light[currentLightID].value = 0;
    } else if (messageString.substring(3,9) == "BRIGHT") {
      light[currentLightID].value += 1;
      if (light[currentLightID].value > 100) light[currentLightID].value = 100;
    }
  }
  // if this light responded to the message, update status
  light[currentLightID].updateValues();
}


void updateInterrupts() {
  if (rainbowMode) {
    // show startup sequence
    for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
      for (int q=0; q < 3; q++) {
        for (int i = 0; i < LIGHT_COUNT; i++) {
          light[i].theaterChaseRainbow(j, q);
        }
        pixels.show();
        delay(10);
      }
    }

    // now show red
    for (int j=0; j < 30; j++) {     // increase brightness
      for (int i = 0; i < LIGHT_COUNT; i++) {
        light[i].setColor(j, 0, 0); // set as red color
      }
      pixels.show();
      delay(30);
    }
    rainbowMode = false;
  } else if (timerTicks++ > 1001) {
    // new NTP request every ~30 seconds
    for (int i = 0; i < LIGHT_COUNT; i++) {
      light[i].updateTime();
    }
    utilities.timeUpdate();
    timerTicks = 0;
    pixels.show();
  }
}


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
