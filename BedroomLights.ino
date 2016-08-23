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
}


void controlCallback(char *data, uint16_t len) {
  Serial.print("Received: ");
  Serial.println(data);
  for (int i = 0; i < LIGHT_COUNT; i++) {
    light[i].processMessage(data);
  }
}


/* if we need to add multi-threading later...
void updateInterrupts() {
  if (timerTicks++ > 2001) {
    timerTicks = 0;
    timerTicks = 0;
  } else if (timerTicks % 100 == 0) {
    light[0].update();
    light[1].update();
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
