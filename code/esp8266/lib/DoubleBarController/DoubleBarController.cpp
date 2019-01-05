/*
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

#include "DoubleBarController.h"

// class definitions
DoubleBarController::DoubleBarController(int LEDPin, int NumLEDsPerSide, int NumLEDsBottom) : m_ledpin(LEDPin), m_numledsperside(NumLEDsPerSide), m_numledsbottom(NumLEDsBottom), m_animInterval(1000.0/25.0), m_lastAnimMillis(0), m_updateDisplay(2,true), m_tickerMajor(0), m_tickerMinor(0), m_sideOptions(2), m_LEDColors(2,std::vector<ColorHelper::RGB>(m_numledsperside,{0,0,0})), leds(NumLEDsPerSide*2, LEDPin, NEO_GRB + NEO_KHZ800)
{
  m_sideOptions[0].animation=std::make_shared<Animation::AnimationBase>(floor(m_numledsbottom/2.0),m_numledsperside-floor(m_numledsbottom/2.0));
  m_sideOptions[1].animation=m_sideOptions[0].animation;
}

void DoubleBarController::setup()
{
  leds.begin();
  leds.clear();
  leds.show();
}

void DoubleBarController::clear(BarSide side)
{
  m_sideOptions[side].primaryScale=nullptr;
  m_sideOptions[side].secondaryScale=nullptr;
}

void DoubleBarController::setPrimaryScale(BarSide side, double lowerBoundValue, double upperBoundValue)
{
  if(m_sideOptions[side].primaryScale==nullptr) {
    m_sideOptions[side].primaryScale=std::make_shared<ColorHelper::Scale<int>>(lowerBoundValue,side*m_numledsperside+m_numledsbottom,upperBoundValue,(side+1)*m_numledsperside);
  } else {
    m_sideOptions[side].primaryScale->setLowerBound(lowerBoundValue);
    m_sideOptions[side].primaryScale->setUpperBound(upperBoundValue);
  }
}

void DoubleBarController::setSecondaryScalePrimaryOnly(BarSide side, JsonArray& values, JsonArray& colors)
{
  yield();

  ColorHelper::RGB color1={(uint8_t) colors[0].as<JsonArray>()[0].as<unsigned int>(),(uint8_t) colors[0].as<JsonArray>()[1].as<unsigned int>(),(uint8_t) colors[0].as<JsonArray>()[2].as<unsigned int>()};
  ColorHelper::RGB color2={(uint8_t) colors[colors.size()-1].as<JsonArray>()[0].as<unsigned int>(),(uint8_t) colors[colors.size()-1].as<JsonArray>()[1].as<unsigned int>(),(uint8_t) colors[colors.size()-1].as<JsonArray>()[2].as<unsigned int>()};

  if(m_sideOptions[side].secondaryScale==nullptr) {
    m_sideOptions[side].secondaryScale=std::make_shared<ColorHelper::Scale<ColorHelper::LCH>>(side*m_numledsperside+m_numledsbottom,ColorHelper::RGBtoLCH(color1),(side+1)*m_numledsperside-1,ColorHelper::RGBtoLCH(color2));
  } else {
    m_sideOptions[side].secondaryScale->setLowerBound(side*m_numledsperside+m_numledsbottom, ColorHelper::RGBtoLCH(color1));
    m_sideOptions[side].secondaryScale->setUpperBound((side+1)*m_numledsperside-1, ColorHelper::RGBtoLCH(color2));

    m_sideOptions[side].secondaryScale->clearInnerValues();
  }

  if(values.size()>2) {
    for(unsigned int i=1; i<values.size()-1; i++){
      double value=m_sideOptions[side].secondaryScale->getLowerBoundValue()+(m_sideOptions[side].secondaryScale->getUpperBoundValue()-m_sideOptions[side].secondaryScale->getLowerBoundValue())/abs(values[values.size()-1].as<double>()-values[0].as<double>())*abs(values[i].as<double>()-values[0].as<double>());
      ColorHelper::RGB intermediatecolor={(uint8_t) colors[i].as<JsonArray>()[0].as<unsigned int>(),(uint8_t) colors[i].as<JsonArray>()[1].as<unsigned int>(),(uint8_t) colors[i].as<JsonArray>()[2].as<unsigned int>()};
      m_sideOptions[side].secondaryScale->addInnerValue(value, ColorHelper::RGBtoLCH(intermediatecolor));
    }
  }

  refreshSecondaryLEDColors(side);
}

void DoubleBarController::refreshSecondaryLEDColors(BarSide side)
{
  myLogger.addToLog("Refreshing LEDColors");
  for(int i=side*m_numledsperside; i<(side+1)*m_numledsperside; i++) {
    m_LEDColors[side][i-side*m_numledsperside]=ColorHelper::LCHtoRGB(m_sideOptions[side].secondaryScale->getProperty(i));
  }
}

void DoubleBarController::setSecondaryScaleSecondarySingleColor(BarSide side, JsonArray& values, JsonArray& colors)
{
  yield();

  ColorHelper::RGB color1={(uint8_t) colors[0].as<JsonArray>()[0].as<unsigned int>(),(uint8_t) colors[0].as<JsonArray>()[1].as<unsigned int>(),(uint8_t) colors[0].as<JsonArray>()[2].as<unsigned int>()};
  ColorHelper::RGB color2={(uint8_t) colors[colors.size()-1].as<JsonArray>()[0].as<unsigned int>(),(uint8_t) colors[colors.size()-1].as<JsonArray>()[1].as<unsigned int>(),(uint8_t) colors[colors.size()-1].as<JsonArray>()[2].as<unsigned int>()};

  if(m_sideOptions[side].secondaryScale==nullptr) {
    m_sideOptions[side].secondaryScale=std::make_shared<ColorHelper::Scale<ColorHelper::LCH>>(values[0].as<double>(),ColorHelper::RGBtoLCH(color1),values[values.size()-1].as<double>(),ColorHelper::RGBtoLCH(color2));
  } else {
    m_sideOptions[side].secondaryScale->setLowerBound(values[0].as<double>(), ColorHelper::RGBtoLCH(color1));
    m_sideOptions[side].secondaryScale->setUpperBound(values[values.size()-1].as<double>(), ColorHelper::RGBtoLCH(color2));

    m_sideOptions[side].secondaryScale->clearInnerValues();
  }

  if(values.size()>2) {
    for(unsigned int i=1; i<values.size()-1; i++){
      ColorHelper::RGB intermediatecolor={(uint8_t) colors[i].as<JsonArray>()[0].as<unsigned int>(),(uint8_t) colors[i].as<JsonArray>()[1].as<unsigned int>(),(uint8_t) colors[i].as<JsonArray>()[2].as<unsigned int>()};
      m_sideOptions[side].secondaryScale->addInnerValue(values[i].as<double>(), ColorHelper::RGBtoLCH(intermediatecolor));
    }
  }
}

int DoubleBarController::getPrimaryProperty(BarSide side, double value)
{
  if(m_sideOptions[side].primaryScale!=nullptr) {
    return m_sideOptions[side].primaryScale->getProperty(value);
  } else {
    return 0.0;
  }
}

ColorHelper::LCH DoubleBarController::getSecondaryProperty(BarSide side, double value)
{
  if(m_sideOptions[side].secondaryScale!=nullptr) {
    return m_sideOptions[side].secondaryScale->getProperty(value);
  } else {
    return {0,0,0};
  }
}

void DoubleBarController::handle()
{
  bool changed=false;

  for(int side=0; side<2; side++) {
    if(m_updateDisplay[side]) {
      myLogger.addToLog("Updating display");
      m_updateDisplay[side]=false;
      if(m_sideOptions[side].mode==None) {
        myLogger.addToLog("None");
        for(int i=side*m_numledsperside; i<(side+1)*m_numledsperside; i++) {
          leds.setPixelColor(i,0);
        }
        changed=true;
      } else if(m_sideOptions[side].mode==PrimaryOnly) {
        myLogger.addToLog("Primary Only");
        m_sideOptions[side].colorMemory=m_LEDColors[side][m_sideOptions[side].primaryScale->getProperty(m_sideOptions[side].lastValue)-side*m_numledsperside];
        for(int i=side*m_numledsperside; i<(side+1)*m_numledsperside; i++) {
          if(i<m_sideOptions[side].primaryScale->getProperty(m_sideOptions[side].lastValue)) {
            leds.setPixelColor(i,m_LEDColors[side][i-side*m_numledsperside].R,m_LEDColors[side][i-side*m_numledsperside].G,m_LEDColors[side][i-side*m_numledsperside].B);
          } else {
            leds.setPixelColor(i,0);
          }
        }
        changed=true;
      } else if(m_sideOptions[side].mode==SecondarySingleColor) {
        myLogger.addToLog("Secondary Single Color");
        ColorHelper::RGB secondarycolor=ColorHelper::LCHtoRGB(m_sideOptions[side].secondaryScale->getProperty(m_sideOptions[side].lastSecondaryValue));
        m_sideOptions[side].colorMemory=secondarycolor;
        for(int i=side*m_numledsperside; i<(side+1)*m_numledsperside; i++) {
          if(i<m_sideOptions[side].primaryScale->getProperty(m_sideOptions[side].lastValue)) {
            leds.setPixelColor(i,secondarycolor.R,secondarycolor.G,secondarycolor.B);
          } else {
            leds.setPixelColor(i,0);
          }
        }
        changed=true;
      }
    }
  }

  if(millis()-m_lastAnimMillis>m_animInterval) {
    if((m_sideOptions[0].mode==DisplayMode::SecondaryAnimation && m_sideOptions[0].animation!=nullptr) ||
       (m_sideOptions[1].mode==DisplayMode::SecondaryAnimation && m_sideOptions[1].animation!=nullptr)) {
      int sidestart=1;
      int sideend=1;
      if(m_sideOptions[0].mode==DisplayMode::SecondaryAnimation && m_sideOptions[0].animation!=nullptr) {
        sidestart=0;
        m_sideOptions[0].animation->updateNextFrame(&m_LEDColors,true,((m_sideOptions[1].mode==DisplayMode::SecondaryAnimation) && (m_sideOptions[1].animation==m_sideOptions[0].animation) ? true : false));
        if((m_sideOptions[1].mode==DisplayMode::SecondaryAnimation) && (m_sideOptions[1].animation==m_sideOptions[0].animation)) {
          sideend=2;
        }
      }
      if(m_sideOptions[1].mode==DisplayMode::SecondaryAnimation && m_sideOptions[1].animation!=nullptr && m_sideOptions[1].animation!=m_sideOptions[0].animation) {
        sideend=2;
        m_sideOptions[1].animation->updateNextFrame(&m_LEDColors,false,true);
      }

      for(int side=sidestart; side<sideend; side++) {
        for(int i=side*m_numledsperside; i<(side+1)*m_numledsperside; i++) {
          if(i<m_sideOptions[side].primaryScale->getProperty(m_sideOptions[side].lastValue)) {
            leds.setPixelColor(i,m_LEDColors[side][i-side*m_numledsperside].R,m_LEDColors[side][i-side*m_numledsperside].G,m_LEDColors[side][i-side*m_numledsperside].B);
          } else {
            leds.setPixelColor(i,0);
          }
        }
      }
      changed=true;
    }

    m_tickerMinor++;
    if(m_tickerMinor>=6) {
      m_tickerMajor++;
      m_tickerMinor=0;
    }

    for(int side=0; side<2; side++) {
      if(m_sideOptions[side].mode!=None) {
        int LEDid=m_sideOptions[side].primaryScale->getProperty(m_sideOptions[side].lastValue);
        if(m_tickerMajor%2==0 && (m_tickerMajor/2)<m_sideOptions[side].numFlashes && !m_sideOptions[side].flashState) {
          // blank the last LED that should be on
          if(m_sideOptions[side].mode==PrimaryOnly || m_sideOptions[side].mode==SecondarySingleColor) {
            leds.setPixelColor(LEDid,m_sideOptions[side].colorMemory.R,m_sideOptions[side].colorMemory.G,m_sideOptions[side].colorMemory.B);
            changed=true;
          }
          m_sideOptions[side].flashState=true;
        } else if((m_tickerMajor%2==1 || (m_tickerMajor/2)>=m_sideOptions[side].numFlashes) && m_sideOptions[side].flashState) {
          // led should be on. reset if not already covered by animation
          leds.setPixelColor(LEDid,0);
          m_sideOptions[side].flashState=false;
          changed=true;
        }
      }
    }

    if(m_tickerMajor>=16) {
      m_tickerMajor=0;
    }

    m_lastAnimMillis=millis();
  }

  if(changed) leds.show();
}

void DoubleBarController::displayValue(BarSide side, double value)
{
  myLogger.addToLog("DisplayValue: "+String(value)+" on side: "+String(side));
  m_sideOptions[side].lastValue=value;

  if(value<m_sideOptions[side].primaryScale->getLowerBoundValue() || value>m_sideOptions[side].primaryScale->getUpperBoundValue()) {
    m_sideOptions[side].numFlashes=0;
  } else {
    double delta=(m_sideOptions[side].primaryScale->getUpperBoundValue()-m_sideOptions[side].primaryScale->getLowerBoundValue())/double(m_sideOptions[side].primaryScale->getUpperBoundProperty()-m_sideOptions[side].primaryScale->getLowerBoundProperty());
    double lowVal=(m_sideOptions[side].lastValue-m_sideOptions[side].primaryScale->getLowerBoundValue());

    int multiplicator=floor(lowVal/delta);
    double currentVal=lowVal-multiplicator*delta;
    m_sideOptions[side].numFlashes=floor(currentVal/delta*4);
  }

  m_sideOptions[side].flashState=false;

  m_updateDisplay[side]=true;
}

void DoubleBarController::displaySecondaryValue(BarSide side, double value)
{
  myLogger.addToLog("DisplaySecondaryValue: "+String(value)+" on side: "+String(side));
  m_sideOptions[side].lastSecondaryValue=value;
  m_updateDisplay[side]=true;
}

void DoubleBarController::displayAnimation(BarSide side, Animation::AnimationType animationtype)
{
  if(m_sideOptions[side].animation->getType()!=animationtype) {
    if(m_sideOptions[(side+1)%2].animation->getType()==animationtype) {
      m_sideOptions[side].animation=m_sideOptions[(side+1)%2].animation;
    } else {
      switch(animationtype) {
        case Animation::AnimationType::AnimationNone:
          m_sideOptions[side].animation=std::make_shared<Animation::AnimationBase>(floor(m_numledsbottom/2.0),m_numledsperside-floor(m_numledsbottom/2.0));
          break;
        case Animation::AnimationType::Fire:
          m_sideOptions[side].animation=std::make_shared<Animation::AnimationFire>(floor(m_numledsbottom/2.0),m_numledsperside-floor(m_numledsbottom/2.0));
          break;
        case Animation::AnimationType::LightRain:
          m_sideOptions[side].animation=std::make_shared<Animation::AnimationRain>(floor(m_numledsbottom/2.0),m_numledsperside-floor(m_numledsbottom/2.0));
          break;
        case Animation::AnimationType::LightRainLightning:
          m_sideOptions[side].animation=std::make_shared<Animation::AnimationRain>(floor(m_numledsbottom/2.0),m_numledsperside-floor(m_numledsbottom/2.0), false, true);
          break;
        case Animation::AnimationType::HeavyRain:
          m_sideOptions[side].animation=std::make_shared<Animation::AnimationRain>(floor(m_numledsbottom/2.0),m_numledsperside-floor(m_numledsbottom/2.0), true);
          break;
        case Animation::AnimationType::HeavyRainLightning:
          m_sideOptions[side].animation=std::make_shared<Animation::AnimationRain>(floor(m_numledsbottom/2.0),m_numledsperside-floor(m_numledsbottom/2.0), true, true);
          break;
        case Animation::AnimationType::Snow:
          m_sideOptions[side].animation=std::make_shared<Animation::AnimationSnow>(floor(m_numledsbottom/2.0),m_numledsperside-floor(m_numledsbottom/2.0));
          break;
        case Animation::AnimationType::Sunny:
          m_sideOptions[side].animation=std::make_shared<Animation::AnimationSun>(floor(m_numledsbottom/2.0),m_numledsperside-floor(m_numledsbottom/2.0));
          break;
        case Animation::AnimationType::SunnyPartlyCloudy:
          m_sideOptions[side].animation=std::make_shared<Animation::AnimationSun>(floor(m_numledsbottom/2.0),m_numledsperside-floor(m_numledsbottom/2.0),5);
          break;
        case Animation::AnimationType::SunnyCloudy:
          m_sideOptions[side].animation=std::make_shared<Animation::AnimationSun>(floor(m_numledsbottom/2.0),m_numledsperside-floor(m_numledsbottom/2.0),20);
          break;
      }
    }
  }
}

bool DoubleBarController::setMode(BarSide side, DisplayMode mode)
{
  if(mode==None || mode==SecondaryAnimation) {
    m_sideOptions[side].mode=mode;
  } else if(mode==PrimaryOnly) {
    myLogger.addToLog("Primary only");
    if(m_sideOptions[side].primaryScale!=nullptr) {
      myLogger.addToLog("RefreshColors");
      refreshSecondaryLEDColors(side);
      m_sideOptions[side].mode=mode;
    } else {
      return false;
    }
  } else if(mode==SecondarySingleColor) {
    if(m_sideOptions[side].secondaryScale!=nullptr) {
      m_sideOptions[side].mode=mode;
    } else {
      return false;
    }
  }
  m_updateDisplay[side]=true;
  return true;
}

void DoubleBarController::setBrightness(uint8_t brightness)
{
  //myLogger.addToLog("Setting brightness to "+String(brightness));
  leds.setBrightness(brightness);
  for(int side=0; side<2; side++) {
    m_updateDisplay[side]=true;
  }
}

uint8_t DoubleBarController::getBrightness()
{
  return leds.getBrightness();
}
