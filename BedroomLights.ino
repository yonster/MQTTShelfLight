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
bool startingUp = true;            // initiates startup sequence

// create objects
#define LIGHT_COUNT 3
Light light[LIGHT_COUNT];
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

  pixels.begin();  // This initializes the NeoPixel library.
  pixels.show();   // Initialize all pixels to 'off'
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
}


void controlCallback(char *data, uint16_t len) {
  Serial.print("Received: ");
  Serial.println(data);
  for (int i = 0; i < LIGHT_COUNT; i++) {
    light[i].processMessage(data);
  }
}


void updateInterrupts() {
  if (light[2].value == 1) {
    // if third light at 1%, show time
    if (timerTicks++ > 10001) {
      // new NTP request every 10 seconds
      utilities.timeUpdate();
      timerTicks = 0;
    } else if (timerTicks % 1000 == 0) {
      // check for a NTP response every second if needed
      utilities.timeCheckup();
    }
  } else if ((startingUp) || (light[1].value == 4)) {
    // show startup sequence
    for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
      for (int q=0; q < 3; q++) {
        light[2].theaterChaseRainbow(j, q);
        pixels.show();
        delay(5);
     }
    }
    startingUp = false;
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
