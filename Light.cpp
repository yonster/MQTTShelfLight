#include "Light.h"

/*
  Light.cpp - drives Neopixel object
  Created by Ian Steyaert, Dec 13, 2015.
  Released into the public domain.

  Control a RGB led with Hue, Saturation and Brightness (HSB / HSV )

  Hue is change by an analog input.
  Brightness is changed by a fading function.
  Saturation stays constant at 255

  getRGB() function based on <http://www.codeproject.com/miscctrl/CPicker.asp>
  dim_curve idea by Jims <http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1174674545>
*/

Light::Light() {
}

//void Light::setup(int startPixel, int endPixel, int lightID, char* lightIDString, GeneralNeoPixelFunction setPixelColor) {
void Light::setup(int startPixel, int endPixel, int lightID, char* _lightIDString, GeneralNeoPixelFunction setPixelColor, GeneralMessageFunction callback) {
  _startPixel = startPixel;
  _endPixel = endPixel;
  _lightID = lightID;
  _setPixelColor = setPixelColor;
  _callback = callback;
  lightIDString = _lightIDString;


//  for {i=_startPixel; i<= _endPixel; i++) {
//    hueCurrent[i] = 0;
//    satCurrent[i] = 0;
//    valCurrent[i] = 0;
//    hueTarget[i] = 0;
//    satTarget[i] = 0;
//    valTarget[i] = 0;
//  }
}

void Light::updateValues() {
  /// need to fix this:
//  if (updateRequired) {
//    for(int i=_startPixel;i<=_endPixel;i++) {
//      if (hueTarget[i] - hueCurrent[i] > 0) {
//        // update
//      } else {
//        updateRequired = false;
//      }
//    }
//  }

  getRGB(hue, saturation, value, _rgb_colors);   // converts HSB to RGB

  if (status == "ON") {
    for(int i=_startPixel;i<=_endPixel;i++) {
      _setPixelColor(i, _rgb_colors[0], _rgb_colors[1], _rgb_colors[2], _rgb_colors[3]);  ////// <<<<<<<< update for WHITE
    }
  } else {
    for(int i=_startPixel;i<=_endPixel;i++) {
      _setPixelColor(i, 0, 0, 0, 0);
    }
  }

  // set updated status
  _callback(String(lightIDString)+":"+String(status)+":"+String(value)+":"+String(hue)+":"+String(saturation));
}

void Light::updateTime() {
  if (clockMode) {
    // update time display
    // turn off all LEDs
    for(int i=_startPixel; i<=_endPixel; i++) {
      _setPixelColor(i, 0, 0, 0, 0);
    }
  
  	// time hours
   int currentHour = hour();
   if (currentHour > 12) { currentHour =- 12; }
   
    for(int i=0;i<=11;i++) {
      // for 12 pixels, color them dark red, except current hour
      if (i == currentHour - 1) {
        _setPixelColor(_endPixel - i, 2, 0, 0, 0);
      } else {
        _setPixelColor(_endPixel - i, 1, 0, 0, 0);
      }
    }
  
    // time minutes
    int currentQuarter = 3 - floor(minute()/15);  // calculate current quarter hour 0 -> 3, reversed
    
    for(int i=0;i<=3;i++) {
      // for 4 pixels, color them dark red, except current minute
      if (i == currentQuarter) {
        _setPixelColor(_startPixel + i, 2, 0, 0, 0);
      } else {
        _setPixelColor(_startPixel + i, 1, 0, 0, 0);
      }
    }
  }
}


void Light::toggle() {
  if (status == "ON") {
    status = "OFF";
  } else {
    status = "ON";
  }
  updateValues();
}


//Theatre-style crawling lights with rainbow effect
void Light::theaterChaseRainbow(int j, int q) {
  for (uint16_t i=_startPixel; i <= _endPixel; i++) {
    byte WheelPos = 255 - (i+j) % 255;
    if (WheelPos < 85) {
      _setPixelColor(i+q, 255 - WheelPos * 3, 0, WheelPos * 3, 0);
    } else if (WheelPos < 170) {
      WheelPos -= 85;
      _setPixelColor(i+q, 0, WheelPos * 3, 255 - WheelPos * 3, 0);
    } else {
      WheelPos -= 170;
      _setPixelColor(i+q, WheelPos * 3, 255 - WheelPos * 3, 0, 0);
    }
  }
}


void Light::setColor(int R, int G, int B, int W) {
  for (uint16_t i=_startPixel; i <= _endPixel; i++) {
    _setPixelColor(i, R, G, B, W);
  }
  hue = 0;
  saturation = 0;
  value = W;
}


//Theatre-style crawling lights with rainbow effect
void Light::getRGB(int _hue, int _sat, int _val, int colors[3]) {
  /* convert hue, saturation and brightness ( HSB/HSV ) to RGB
     The dim_curve is used only on brightness/value and on saturation (inverted).
     This looks the most natural.
  */

  _val = dim_curve[_val];
  _sat = 255-dim_curve[255-_sat];

  int r;
  int g;
  int b;
  int base;

  if (_sat == 0) { // Acromatic color (gray). Hue doesn't mind.
    colors[0]=_val;
    colors[1]=_val;
    colors[2]=_val;
    colors[3]=_val;
  } else  {

    base = ((255 - _sat) * _val)>>8;

    switch(_hue/60) {
  case 0:
    r = _val;
    g = (((_val-base)*_hue)/60)+base;
    b = base;
  break;

  case 1:
    r = (((_val-base)*(60-(_hue%60)))/60)+base;
    g = _val;
    b = base;
  break;

  case 2:
    r = base;
    g = _val;
    b = (((_val-base)*(_hue%60))/60)+base;
  break;

  case 3:
    r = base;
    g = (((_val-base)*(60-(_hue%60)))/60)+base;
    b = _val;
  break;

  case 4:
    r = (((_val-base)*(_hue%60))/60)+base;
    g = base;
    b = _val;
  break;

  case 5:
    r = _val;
    g = base;
    b = (((_val-base)*(60-(_hue%60)))/60)+base;
  break;
    }

    colors[0]=r;
    colors[1]=g;
    colors[2]=b;
    colors[3]=_val;
  }
}
