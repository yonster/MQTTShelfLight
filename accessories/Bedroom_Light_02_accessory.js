var name = "Bedroom Light 02";
var lightIDString = "02"
var serialNumber = "LGHT0002";
var userName = "1A:2B:3C:4D:3E:FE";

// MQTT Setup
var mqtt = require('mqtt');
var options = {
  port: 1883,
  host: '127.0.0.1',
  clientId: 'Steyaert_MQTT_02'
};
var client = mqtt.connect(options);

client.on('connect', function () {
  console.log("subscribing to status feed");
  client.subscribe('steyaertHome/masterBedroom/lightsStatus');
//  client.publish('steyaertHome/masterBedroom/lightsStatus', 'Hello mqtt');
});

client.on('message', function (topic, message) {
  // message is Buffer
  if (message != "") {
    var status = String(message).split(':');
    if (status[0] == lightIDString) {
      console.log("%s rcvd %s", name, message);
      switch (status[1]) {
        case "HUE":
          BEDROOM_LIGHT.hue = parseFloat(status[2]);
          break;
        case "SAT":
          BEDROOM_LIGHT.saturation = parseFloat(status[2]);
          break;
        case "VAL":
          BEDROOM_LIGHT.brightness = parseFloat(status[2]);
          break;
        case "ON":
          BEDROOM_LIGHT.powerOn = (status[2] == "ON");
      }
    }
  }
});

var Accessory = require('../').Accessory;
var Service = require('../').Service;
var Characteristic = require('../').Characteristic;
var uuid = require('../').uuid;

// here's a fake hardware device that we'll expose to HomeKit
var BEDROOM_LIGHT = {
  powerOn: false,
  brightness: 100, // percentage
  hue: 100,
  saturation: 100,

  setPowerOn: function(on) {
    console.log("Turning %s %s", name, on ? "on" : "off");
    BEDROOM_LIGHT.powerOn = on;

    if (on) {
            client.publish('steyaertHome/masterBedroom/lightsControl', lightIDString.concat(':ON'));
    } else {
            client.publish('steyaertHome/masterBedroom/lightsControl', lightIDString.concat(':OFF'));
    };
  },
  setBrightness: function(brightness) {
    console.log("Setting light brightness to %s", brightness);
    BEDROOM_LIGHT.brightness = brightness;
    client.publish('steyaertHome/masterBedroom/lightsControl', lightIDString.concat(':VAL:', brightness));
  },
  setHue: function(hue) {
    console.log("Setting light hue to %s", hue);
    BEDROOM_LIGHT.hue = hue;
    client.publish('steyaertHome/masterBedroom/lightsControl', lightIDString.concat(':HUE:', hue));
  },
  setSaturation: function(saturation) {
    console.log("Setting light brightness to %s", saturation);
    BEDROOM_LIGHT.saturation = saturation;
    client.publish('steyaertHome/masterBedroom/lightsControl', lightIDString.concat(':SAT:', saturation));
  },
  identify: function() {
    console.log("Identify %s", name);
    client.publish('steyaertHome/masterBedroom/lightsControl', lightIDString.concat(':IDENTIFY'));
  },
  checkStatus: function() {
    console.log("Checking %s status", name);
    client.publish('steyaertHome/masterBedroom/lightsControl', lightIDString.concat(':CHECKSTATUS'));
  }
}

// Generate a consistent UUID for our light Accessory that will remain the same even when
// restarting our server. We use the `uuid.generate` helper function to create a deterministic
// UUID based on an arbitrary "namespace" and the word "light".
var lightUUID = uuid.generate('hap-nodejs:accessories:light');

// This is the Accessory that we'll return to HAP-NodeJS that represents our fake light.
var light = exports.accessory = new Accessory(name, lightUUID);

// delay function to wait for response
function delay(ms) {
  var start_time = Date.now();
  while (Date.now() - start_time < ms);
}

// Add properties for publishing (in case we're using Core.js and not BridgedCore.js)
light.username = userName;
light.pincode = "031-45-154";

// set some basic properties (these values are arbitrary and setting them is optional)
light
  .getService(Service.AccessoryInformation)
  .setCharacteristic(Characteristic.Manufacturer, "Steyaert")
  .setCharacteristic(Characteristic.Model, "Rev-1")
  .setCharacteristic(Characteristic.SerialNumber, serialNumber);

// listen for the "identify" event for this Accessory
light.on('identify', function(paired, callback) {
  BEDROOM_LIGHT.identify();
  callback(); // success
});

// Add the actual Lightbulb Service and listen for change events from iOS.
// We can see the complete list of Services and Characteristics in `lib/gen/HomeKitTypes.js`
light
  .addService(Service.Lightbulb, name) // services exposed to the user should have "names" like "Fake Light" for us
  .getCharacteristic(Characteristic.On)
  .on('set', function(value, callback) {
    BEDROOM_LIGHT.setPowerOn(value);
    callback(); // Our fake Light is synchronous - this value has been successfully set
  });

// We want to intercept requests for our current power state so we can query the hardware itself instead of
// allowing HAP-NodeJS to return the cached Characteristic.value.
light
  .getService(Service.Lightbulb)
  .getCharacteristic(Characteristic.On)
  .on('get', function(callback) {

    // this event is emitted when you ask Siri directly whether your light is on or not. you might query
    // the light hardware itself to find this out, then call the callback. But if you take longer than a
    // few seconds to respond, Siri will give up.

    var err = null; // in case there were any problems
    BEDROOM_LIGHT.checkStatus();

    // wait a bit
    delay(200);

    if (BEDROOM_LIGHT.powerOn) {
      console.log("Are we on? Yes.");
      callback(err, true);
    }
    else {
      console.log("Are we on? No.");
      callback(err, false);
    }
  });

// also add an "optional" Characteristic for Brightness
light
  .getService(Service.Lightbulb)
  .addCharacteristic(Characteristic.Brightness)
  .on('get', function(callback) {
    callback(null, BEDROOM_LIGHT.brightness);
  })
  .on('set', function(value, callback) {
    BEDROOM_LIGHT.setBrightness(value);
    callback();
  })

light
  .getService(Service.Lightbulb)
  .addCharacteristic(Characteristic.Hue)
  .on('get', function(callback) {
    callback(null, BEDROOM_LIGHT.hue);
  })
  .on('set', function(value, callback) {
    BEDROOM_LIGHT.setHue(value);
    callback();
  })

light
  .getService(Service.Lightbulb)
  .addCharacteristic(Characteristic.Saturation)
  .on('get', function(callback) {
    callback(null, BEDROOM_LIGHT.saturation);
  })
  .on('set', function(value, callback) {
    BEDROOM_LIGHT.setSaturation(value);
    callback();
  })
