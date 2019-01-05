/* DoubleBarController.h

Purpose
=======
DoubleBarController.h is the display module of the ThermoLight that controls the NeoPixels with the Adafruit_NeoPixel library.
The number of LEDs in the bar graph are configurable, however configurations other than the standard one have not been tested.

Animations can currently not be configured for the ThermoLight, however in principle do work.

License
=======

MIT License

Copyright (c) 2019 Harald Braun

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef DoubleBarController_h
#define DoubleBarController_h

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

#include <memory>
#include <Adafruit_NeoPixel.h>
#include <ColorHelper.hpp>
#include <AnimationsBundle.hpp>
#include <ArduinoJson.h>
#include <Logger.h>

class DoubleBarController
{
public:

  typedef enum {
    Left=0,
    Right=1
  } BarSide;

  typedef enum {
    None,
    PrimaryOnly,
    SecondarySingleColor,
    SecondaryAnimation
  } DisplayMode;

  DoubleBarController(int LEDPin, int NumLEDsPerSide, int NumLEDsBottom);
  void setup();
  void handle();

  void setPrimaryScale(BarSide side, double lowerBoundValue, double upperBoundValue);
  void setSecondaryScalePrimaryOnly(BarSide side, JsonArray& values, JsonArray& colors);
  void setSecondaryScaleSecondarySingleColor(BarSide side, JsonArray& values, JsonArray& colors);
  void clear(BarSide side);

  int getPrimaryProperty(BarSide side, double value);
  ColorHelper::LCH getSecondaryProperty(BarSide side, double value);

  void displayValue(BarSide side, double value);
  void displaySecondaryValue(BarSide side, double value);

  void displayAnimation(BarSide side, Animation::AnimationType animationtype);

  bool setMode(BarSide side, DisplayMode mode);

  void setBrightness(uint8_t brightness);
  uint8_t getBrightness();

private:
  struct SideOptions {
  public:
    SideOptions() : mode(None), primaryScale(nullptr), secondaryScale(nullptr), animation(nullptr), lastValue(0), lastSecondaryValue(0), numFlashes(0), flashState(false), colorMemory({0,0,0}) {};
    DisplayMode mode;
    std::shared_ptr<ColorHelper::Scale<int>> primaryScale;
    std::shared_ptr<ColorHelper::Scale<ColorHelper::LCH>> secondaryScale;
    std::shared_ptr<Animation::AnimationBase> animation;
    double lastValue;
    double lastSecondaryValue;
    int numFlashes;
    bool flashState;
    ColorHelper::RGB colorMemory;
  };

  void refreshSecondaryLEDColors(BarSide side);

  int m_ledpin;
  int m_numledsperside;
  int m_numledsbottom;
  unsigned long m_animInterval;
  unsigned long m_lastAnimMillis;
  std::vector<bool> m_updateDisplay;

  int m_tickerMajor;
  int m_tickerMinor;

  std::vector<SideOptions> m_sideOptions;
  std::vector<std::vector<ColorHelper::RGB>> m_LEDColors;

  Adafruit_NeoPixel leds;
};

#endif
