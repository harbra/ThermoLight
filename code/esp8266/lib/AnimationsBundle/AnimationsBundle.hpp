/* AnimationsBundle.hpp

Purpose
=======
AnimationsBundle.hpp provides pre-defined animations for displaying with DoubleBarController.h. The functionality
can currently however not be configured for the ThermoLight.

As a basis, the fire animation from the FastLED library was used. Then custom functions were added to
be able to provide animations for common weather conditions:
* LightRain,
* LightRainLightning,
* HeavyRain,
* HeavyRainLightning,
* Snow,
* Sunny,
* SunnyPartlyCloudy,
* SunnyCloudy

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

#ifndef AnimationsBundle_h
#define AnimationsBundle_h

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

#include <Logger.h>
#include <ColorHelper.hpp>

namespace Animation
{
  typedef enum {
    AnimationNone,
    Fire,
    LightRain,
    LightRainLightning,
    HeavyRain,
    HeavyRainLightning,
    Snow,
    Sunny,
    SunnyPartlyCloudy,
    SunnyCloudy
  } AnimationType;

  class AnimationParticleRain
  {
  public:
    uint8_t x;
    double y;
    uint8_t speed_y;
  };

  class AnimationParticleCloud
  {
  public:
    double x;
    double y;
    uint8_t speed_x;
    uint8_t size_x;
    uint8_t size_y;
    uint8_t intensity;
  };

  class AnimationParticleSun
  {
  public:
    float color_f; // current factor to adjust center color
    float color_fmax; // max factor to adjust center color
    float color_v; // color change velocity
    bool direction; // direction of color change
  };

  class AnimationBase
  {
  public:
    AnimationBase(int NumOuterLEDs, int NumInnerLEDs) : m_numOuterLEDs(NumOuterLEDs), m_numInnerLEDs(NumInnerLEDs), m_leftUpdated(false), m_rightUpdated(false), m_animationType(AnimationNone) {};

    AnimationType getType()
    {
      return m_animationType;
    };

    virtual void updateNextFrame(std::vector<std::vector<ColorHelper::RGB>>* LEDArray, bool left, bool right)
    {
      if(m_leftUpdated) left=false;
      if(m_rightUpdated) right=false;

      for(int side=(left ? 0 : 1); side<(right ? 2 : 1); side++) {
        for(int i=0; i<m_numOuterLEDs+m_numInnerLEDs; i++) {
          (*LEDArray)[side][i]=ColorHelper::RGB({0,0,0});
        }
      }
    };

  protected:
    AnimationBase(int NumOuterLEDs, int NumInnerLEDs, AnimationType animationtype) : m_numOuterLEDs(NumOuterLEDs), m_numInnerLEDs(NumInnerLEDs), m_leftUpdated(false), m_rightUpdated(false), m_animationType(animationtype) {};

    bool getLEDIndex(int &index, int &side, int x, int y)
    {
      switch(x) {
        case 0:
        {
          index=m_numOuterLEDs-1-y;
          side=0;
          return index>=0 && index<m_numOuterLEDs;
          break;
        }
        case 1:
        {
          index=m_numOuterLEDs+y;
          side=0;
          return y>=0 && y<m_numInnerLEDs;
          break;
        }
        case 2:
        {
          index=m_numOuterLEDs+y;
          side=1;
          return y>=0 && y<m_numInnerLEDs;
          break;
        }
        case 3:
        {
          index=m_numOuterLEDs-1-y;
          side=1;
          return index>=0 && index<m_numOuterLEDs;
          break;
        }
      };

      return false;
    };

    int m_numOuterLEDs;
    int m_numInnerLEDs;

    bool m_leftUpdated;
    bool m_rightUpdated;

    AnimationType m_animationType;
  };

  class AnimationFire : public AnimationBase
  {
    // fire animation is based on FastLED library's Fire2012 example
    // https://github.com/FastLED/FastLED/blob/master/examples/Fire2012/Fire2012.ino

    // COOLING: How much does the air cool as it rises?
    // Less cooling = taller flames.  More cooling = shorter flames.
    // Default 50, suggested range 20-100
    #define COOLING  55

    // SPARKING: What chance (out of 255) is there that a new spark will be lit?
    // Higher chance = more roaring fire.  Lower chance = more flickery fire.
    // Default 120, suggested range 50-200.
    #define SPARKING 70

  public:
    AnimationFire(int NumOuterLEDs, int NumInnerLEDs) : AnimationBase(NumOuterLEDs, NumInnerLEDs, AnimationType::Fire), m_FireScale(0,{0,0,0},255,{255,255,255}) {
      srand(millis());

      m_FireScale.addInnerValue(85,{255,0,0});
      m_FireScale.addInnerValue(170,{255,255,0});

      m_AnimationContainer.push_back(std::vector<uint8_t>(m_numOuterLEDs,0));
      m_AnimationContainer.push_back(std::vector<uint8_t>(m_numInnerLEDs,0));
      m_AnimationContainer.push_back(std::vector<uint8_t>(m_numInnerLEDs,0));
      m_AnimationContainer.push_back(std::vector<uint8_t>(m_numOuterLEDs,0));
    };

    virtual void updateNextFrame(std::vector<std::vector<ColorHelper::RGB>>* LEDArray, bool left, bool right)
    {
      // Step 1.  Cool down every cell a little
      for(unsigned int k=0; k<m_AnimationContainer.size(); k++) {
        for(unsigned int i = 0; i < m_AnimationContainer[k].size(); i++) {
          uint8_t rnd=rand() % (((COOLING * 10) / m_numInnerLEDs) + 2);
          m_AnimationContainer[k][i]-=rnd>m_AnimationContainer[k][i] ? m_AnimationContainer[k][i] : rnd;
        }
      }

      // Step 2.  Heat from each cell drifts 'up' and diffuses a little
      for(int k=m_numInnerLEDs-1; k>=2; k--) {
        if(k<m_numOuterLEDs) {
          m_AnimationContainer[1][k]=(m_AnimationContainer[1][k-1]+m_AnimationContainer[1][k-2]+m_AnimationContainer[2][k-2]+m_AnimationContainer[0][k-2]+m_AnimationContainer[3][k-2])/5;
          m_AnimationContainer[2][k]=(m_AnimationContainer[2][k-1]+m_AnimationContainer[2][k-2]+m_AnimationContainer[1][k-2]+m_AnimationContainer[0][k-2]+m_AnimationContainer[3][k-2])/5;
          m_AnimationContainer[0][k]=(m_AnimationContainer[0][k-1]+m_AnimationContainer[0][k-2]+m_AnimationContainer[3][k-2])/3;
          m_AnimationContainer[3][k]=(m_AnimationContainer[3][k-1]+m_AnimationContainer[3][k-2]+m_AnimationContainer[2][k-2])/3;
        } else {
          m_AnimationContainer[1][k]=(m_AnimationContainer[1][k-1]+m_AnimationContainer[1][k-2]+m_AnimationContainer[2][k-2])/3;
          m_AnimationContainer[2][k]=(m_AnimationContainer[2][k-1]+m_AnimationContainer[2][k-2]+m_AnimationContainer[1][k-2])/3;
        }
      }

      // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
      for(int i=0; i<4; i++) {
        if(rand()%256 < SPARKING ) {
          int y=rand()%5;
          if(y>=m_AnimationContainer[i].size()) continue;
          uint8_t rnd=rand()%95+100;
          m_AnimationContainer[i][y]+=rnd>255-m_AnimationContainer[i][y] ? 255-m_AnimationContainer[i][y] : rnd;
        }
      }

      // Step 4.  Map from heat cells to LED colors
      uint8_t side=0;
      uint8_t idx=0;
      if(!left) {
        side++;
        idx=m_numOuterLEDs;
      }
      for(unsigned int k=(left? 0 : 2); k<m_AnimationContainer.size(); k++) {
        if(k==0 || k==m_AnimationContainer.size()-1) {
          for(int i=m_AnimationContainer[k].size()-1; i>=0; i--) {
            (*LEDArray)[side][idx]=m_FireScale.getProperty(m_AnimationContainer[k][i]);
            idx++;
          }
        } else {
          for(int i=0; i<m_AnimationContainer[k].size(); i++) {
            (*LEDArray)[side][idx]=m_FireScale.getProperty(m_AnimationContainer[k][i]);
            idx++;
          }
        }
        if(k==1) {
          if(!right) break;
          side++;
          idx=m_numOuterLEDs;
        } else if(k==2) {
          idx=0;
        }
      }
    };

  protected:
    std::vector<std::vector<uint8_t>> m_AnimationContainer;
    ColorHelper::Scale<ColorHelper::RGB> m_FireScale;
  };

  class AnimationRain : public AnimationBase
  {
  public:
    AnimationRain(int NumOuterLEDs, int NumInnerLEDs, bool HeavyRain=false, bool Lightning=false) : AnimationBase(NumOuterLEDs, NumInnerLEDs, (HeavyRain ? (Lightning ? AnimationType::HeavyRainLightning : AnimationType::HeavyRain) : (Lightning ? AnimationType::LightRainLightning : AnimationType::LightRain))), m_probabilityNew(HeavyRain ? 200 : 10), m_lightning(Lightning), m_speedDivider(320.0), m_particleColor({0,0,100}), m_backgroundColor({0,0,0})
    {
    };

    virtual void updateNextFrame(std::vector<std::vector<ColorHelper::RGB>>* LEDArray, bool left, bool right)
    {
      if(rand()%256<m_probabilityNew) {
        AnimationParticleRain NewParticle;

        uint8_t r=rand()%(2*(m_numOuterLEDs+m_numInnerLEDs));
        if(r<m_numOuterLEDs) {
          NewParticle.x=0;
        } else if(r<m_numOuterLEDs+m_numInnerLEDs) {
          NewParticle.x=1;
        } else if(r<m_numOuterLEDs+2*m_numInnerLEDs) {
          NewParticle.x=2;
        } else {
          NewParticle.x=3;
        }

        NewParticle.y=(NewParticle.x==0 || NewParticle.x==3) ? m_numOuterLEDs : m_numInnerLEDs;
        NewParticle.speed_y=(256-32)+rand()%32;

        m_particles.push_back(NewParticle);
      }

      if(rand()%256<5 && m_lightning) {
        for(int k=(left ? 0 : 1); k<(right ? LEDArray->size() : LEDArray->size()-1); k++) {
          std::fill((*LEDArray)[k].begin(),(*LEDArray)[k].end(),ColorHelper::RGB({50,50,50}));
        }
      } else {
        for(int k=(left ? 0 : 1); k<(right ? LEDArray->size() : LEDArray->size()-1); k++) {
          std::fill((*LEDArray)[k].begin(),(*LEDArray)[k].end(),m_backgroundColor);
        }
      }

      for(int i=0; i<m_particles.size(); i++) {
        m_particles[i].y-=double(m_particles[i].speed_y)/m_speedDivider;
        if(m_particles[i].y<-1.0) {
          m_particles.erase(m_particles.begin() + i);
          i--;
          continue;
        }

        int y_int=floor(m_particles[i].y);

        int ledindex=0, side=0;
        if(getLEDIndex(ledindex,side,m_particles[i].x,y_int) && ((left && side==0) || (right && side==1))) {
          (*LEDArray)[side][ledindex]=ColorHelper::add(ColorHelper::RGB((*LEDArray)[side][ledindex]),ColorHelper::multiply(m_particleColor,(y_int+1.0)-m_particles[i].y));
        }
        if(getLEDIndex(ledindex,side,m_particles[i].x,y_int+1) && ((left && side==0) || (right && side==1))) {
          (*LEDArray)[side][ledindex]=ColorHelper::add(ColorHelper::RGB((*LEDArray)[side][ledindex]),ColorHelper::multiply(m_particleColor,m_particles[i].y-y_int));
        }
      }
    };

  protected:
    AnimationRain(int NumOuterLEDs, int NumInnerLEDs, AnimationType animationtype, uint8_t ProbabilityNew, bool Lightning, double InverseSpeed, ColorHelper::RGB ParticleColor, ColorHelper::RGB BackgroundColor) : AnimationBase(NumOuterLEDs, NumInnerLEDs, animationtype), m_probabilityNew(ProbabilityNew), m_lightning(Lightning), m_speedDivider(InverseSpeed), m_particleColor(ParticleColor), m_backgroundColor(BackgroundColor)
    {
    };

    std::vector<AnimationParticleRain> m_particles;

    uint8_t m_probabilityNew;
    bool m_lightning;

    double m_speedDivider;
    ColorHelper::RGB m_particleColor;
    ColorHelper::RGB m_backgroundColor;

  };

  class AnimationSnow : public AnimationRain
  {
  public:
    AnimationSnow(int NumOuterLEDs, int NumInnerLEDs) : AnimationRain(NumOuterLEDs, NumInnerLEDs, AnimationType::Snow, 40, false, 2000.0, {50,50,50}, {0,0,0})
    {
    };
  };

  class AnimationSun : public AnimationBase
  {
  public:
    // CloudDensity of 5 is partly cloudy, 25 is heavy clouds, 0 is no clouds
    AnimationSun(int NumOuterLEDs, int NumInnerLEDs, uint8_t CloudDensity=0) : AnimationBase(NumOuterLEDs, NumInnerLEDs, CloudDensity==0 ? AnimationType::Sunny : (CloudDensity>0 && CloudDensity<15 ? AnimationType::SunnyPartlyCloudy : AnimationType::SunnyCloudy)), m_particlesSun((m_numOuterLEDs*2+1)*2), m_particlesCloud(0), m_probabilityNewCloud(CloudDensity), m_centerColor({255,212,0}), m_backgroundColor({21, 21, 80}), m_cloudColor({200,200,200}), m_CloudScale(0,m_backgroundColor,100,m_cloudColor)
    {
      for(int i=0; i<m_particlesSun.size(); i++) {
        m_particlesSun[i].color_f=1.0;
        m_particlesSun[i].color_fmax=1+(1+rand()%255/1000.0);
        m_particlesSun[i].color_v=(1+rand()%255)/3000.0;
        m_particlesSun[i].direction=rand()%2;
      }
    };

    virtual void updateNextFrame(std::vector<std::vector<ColorHelper::RGB>>* LEDArray, bool left, bool right)
    {
      for(int k=(left ? 0 : 1); k<(right ? LEDArray->size() : LEDArray->size()-1); k++) {
        std::fill((*LEDArray)[k].begin(),(*LEDArray)[k].end(),m_backgroundColor);
      }

      if(rand()%256<m_probabilityNewCloud) {
        AnimationParticleCloud newCloud;
        newCloud.size_x=2*(1+rand()%4);
        newCloud.size_y=2*(1+rand()%3);
        newCloud.x=-newCloud.size_x/2.0;
        newCloud.y=rand()%m_numInnerLEDs;
        newCloud.speed_x=(256-32)+rand()%32;
        newCloud.intensity=32+rand()%64;
        m_particlesCloud.push_back(newCloud);
      }

      std::vector<std::vector<uint8_t>> LEDBuffer;
      for(int i=0; i<4; i++) LEDBuffer.push_back(std::vector<uint8_t>(m_numInnerLEDs,0));

      for(int i=0; i<m_particlesCloud.size(); i++) {
        m_particlesCloud[i].x+=double(m_particlesCloud[i].speed_x)/2560.0;
        if(m_particlesCloud[i].x>4+m_particlesCloud[i].size_x/2.0) {
          m_particlesCloud.erase(m_particlesCloud.begin() + i);
          i--;
          continue;
        }

        double weight_x=(floor(m_particlesCloud[i].x)+1.0)-m_particlesCloud[i].x;
        double weight_xplus=1.0-weight_x;
        double weight_y=(floor(m_particlesCloud[i].y)+1.0)-m_particlesCloud[i].y;
        double weight_yplus=1.0-weight_y;

        int x_low=floor(m_particlesCloud[i].x-m_particlesCloud[i].size_x/2);
        int y_low=floor(m_particlesCloud[i].y-m_particlesCloud[i].size_y/2);
        for(int x=x_low; x<=x_low+m_particlesCloud[i].size_x; x++) {
          for(int y=y_low; y<=y_low+m_particlesCloud[i].size_y; y++) {
            for(int dx=0; dx<=1; dx++) {
              for(int dy=0; dy<=1; dy++) {
                if(x+dx>=0 && x+dx<4 && y+dy>=0 && y+dy<m_numInnerLEDs) {
                  LEDBuffer[x+dx][y+dy]+=m_particlesCloud[i].intensity*(dx ? weight_xplus : weight_x)*(dy ? weight_yplus : weight_y);
                }
              }
            }
          }
        }
      }

      for(int i=0; i<m_particlesSun.size(); i++) {
        if(m_particlesSun[i].direction) {
          m_particlesSun[i].color_f=min(m_particlesSun[i].color_fmax,m_particlesSun[i].color_f+m_particlesSun[i].color_v);
        } else {
          //m_particlesSun[i].color_f=max(2-m_particlesSun[i].color_fmax,m_particlesSun[i].color_f-m_particlesSun[i].color_v);
          m_particlesSun[i].color_f=max(1.f,m_particlesSun[i].color_f-m_particlesSun[i].color_v);
        }
        if(m_particlesSun[i].color_f>=m_particlesSun[i].color_fmax || m_particlesSun[i].color_f<=1.f) {
          m_particlesSun[i].direction=!m_particlesSun[i].direction;
        }

        int side=i<(m_numOuterLEDs*2+1) ? 0 : 1;
        int ledindex=i-side*(m_numOuterLEDs*2+1);
        if((left && side==0) || (right && side==1)) {
          (*LEDArray)[side][ledindex]=ColorHelper::multiply(m_centerColor,m_particlesSun[i].color_f);
        }
      }

      for(int x=0; x<4; x++) {
        for(int y=0; y<m_numInnerLEDs; y++) {
          int ledindex=0, side=0;
          if(getLEDIndex(ledindex,side,x,y) && ((left && side==0) || (right && side==1))) {
            (*LEDArray)[side][ledindex]=ColorHelper::average((*LEDArray)[side][ledindex],m_CloudScale.getProperty(LEDBuffer[x][y]));
          }
        }
      }
    };

  protected:
    std::vector<AnimationParticleSun> m_particlesSun;
    std::vector<AnimationParticleCloud> m_particlesCloud;
    uint8_t m_probabilityNewCloud;
    ColorHelper::RGB m_centerColor;
    ColorHelper::RGB m_backgroundColor;
    ColorHelper::RGB m_cloudColor;
    ColorHelper::Scale<ColorHelper::RGB> m_CloudScale;
  };

};


#endif
