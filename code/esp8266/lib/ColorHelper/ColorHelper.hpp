/* ColorHelper.hpp

Purpose
=======
ColorHelper.hpp provides functions for interpolation between two or more values and is thus used e.g.
for color interpolation of the bar graph.

It also supports color conversion from/to RGB and LCH. This is because LCH interpolation obeys luminosity and
thus ensures that all colors in a gradient are perceveid to be the same "intensity".

Formulas for color space conversion from http://www.brucelindbloom.com/index.html?Math.html

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

#ifndef ColorHelper_h
#define ColorHelper_h

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

#include <math.h>

class ColorHelper
{
public:

  template<class T>
  struct ScaleElement
  {
    double value;
    T property;
  };

  template<class T>
  class Scale
  {
    public:
      Scale(double lowerBound, T lowerBoundValue, double upperBound, T upperBoundValue) : m_lowerBound{lowerBound, lowerBoundValue}, m_upperBound{upperBound, upperBoundValue} {};

      void setLowerBound(double lowerBoundValue) { m_lowerBound.value=lowerBoundValue; };
      void setLowerBound(double lowerBoundValue, T lowerBoundProperty) { m_lowerBound.value=lowerBoundValue;
                                                                         m_lowerBound.property=lowerBoundProperty; };
      void setLowerBoundProperty(T lowerBoundProperty) { m_lowerBound.property=lowerBoundProperty; };
      double getLowerBoundValue() { return m_lowerBound.value; };
      T getLowerBoundProperty() {return m_lowerBound.property; };

      void setUpperBound(double upperBoundValue) { m_upperBound.value=upperBoundValue; };
      void setUpperBound(double upperBoundValue, T upperBoundProperty) { m_upperBound.value=upperBoundValue;
                                                                         m_upperBound.property=upperBoundProperty; };
      void setUpperBoundProperty(T upperBoundProperty) { m_upperBound.property=upperBoundProperty; };
      double getUpperBoundValue() { return m_upperBound.value; };
      T getUpperBoundProperty() {return m_upperBound.property; };

      void clearInnerValues() { m_innerValues.clear(); };
      bool addInnerValue(double value, T property)
      {
        if(value<m_lowerBound.value || value>m_upperBound.value)
        {
          return false;
        }
        for(auto it = m_innerValues.begin(); it<=m_innerValues.end(); it++) {
          if(it==m_innerValues.end())
          {
            m_innerValues.push_back(ScaleElement<T>{value,property});
            break;
          }
          if (value<(*it).value)
          {
            m_innerValues.insert(it, ScaleElement<T>{value, property});
            break;
          }
        }

        return true;
      };
      T getProperty(double value)
      {
        // if value is smaller than lower bound or larger than upper bound, just return the respective edge value.
        if(value<=m_lowerBound.value) {
          return m_lowerBound.property;
        } else if(value>=m_upperBound.value) {
          return m_upperBound.property;
        }

        // if no inner values are given, just interpolate between the bounds
        if(m_innerValues.size()==0) {
          return getInterpolatedProperty(value, m_lowerBound.value, m_lowerBound.property, m_upperBound.value, m_upperBound.property);
        }

        // value must be somewhere in-between
        ScaleElement<T>* it_before=&m_lowerBound;
        for(auto it = m_innerValues.begin(); it!=m_innerValues.end(); it++) {
          if(value<it->value) {
            return getInterpolatedProperty(value, it_before->value, it_before->property, it->value, it->property);
          } else if (value==it->value) {
            return it->property;
          }
          it_before=&*it;
        }
        return getInterpolatedProperty(value, m_innerValues.rbegin()->value, m_innerValues.rbegin()->property, m_upperBound.value, m_upperBound.property);
      };

    protected:
      ScaleElement<T> m_lowerBound;
      ScaleElement<T> m_upperBound;
      std::vector<ScaleElement<T>> m_innerValues;

      T getInterpolatedProperty(double value, double lowerBoundValue, T lowerBoundProperty, double upperBoundValue, T upperBoundProperty)
      {
        // easiest case is linear interpolation
        return lowerBoundProperty+(upperBoundProperty-lowerBoundProperty)*(value-lowerBoundValue)/(upperBoundValue-lowerBoundValue);
      };

      double getInterpolatedPropertyDouble(double value, double lowerBoundValue, double lowerBoundProperty, double upperBoundValue, double upperBoundProperty)
      {
        // easiest case is linear interpolation
        return lowerBoundProperty+(upperBoundProperty-lowerBoundProperty)*(value-lowerBoundValue)/(upperBoundValue-lowerBoundValue);
      };

      double getInterpolatedPropertyHue(double value, double lowerBoundValue, double lowerBoundProperty, double upperBoundValue, double upperBoundProperty)
      {
        //if(upperBoundProperty-lowerBoundProperty>M_PI) lowerBoundProperty+=2*M_PI;
        //else if(lowerBoundProperty-upperBoundProperty>M_PI) upperBoundProperty+=2*M_PI;

        double interpolated=getInterpolatedPropertyDouble(value, lowerBoundValue, lowerBoundProperty, upperBoundValue, upperBoundProperty);

        return interpolated>2*M_PI ? interpolated-2*M_PI : interpolated;
      };
  };

  struct RGB
  {
    uint8_t R;
    uint8_t G;
    uint8_t B;
    String toString() {
      return String("{")+String(R)+","+String(G)+","+String(B)+String("}");
    }
    String toHexString() {
      String r=String(R,HEX);
      String g=String(G,HEX);
      String b=String(B,HEX);

      return String("#")+(R<16 ? String("0") : String(""))+r+(G<16 ? String("0") : String(""))+g+(B<16 ? String("0") : String(""))+b;
    }
  };

  struct XYZ
  {
    double X;
    double Y;
    double Z;
    String toString() {
      return String("{")+String(X)+","+String(Y)+","+String(Z)+String("}");
    }
  };

  struct Lab
  {
    double L;
    double a;
    double b;
    String toString() {
      return String("{")+String(L)+","+String(a)+","+String(b)+String("}");
    }
  };

  struct LCH
  {
    double L;
    double C;
    double H;
    String toString() {
      return String("{")+String(L)+","+String(C)+","+String(H*180.0/M_PI)+String("}");
    }
  };

  static RGB add(RGB lhs, RGB rhs)
  {
    return {min(lhs.R+rhs.R,255),min(lhs.G+rhs.G,255),min(lhs.B+rhs.B,255)};
  };

  static RGB average(RGB lhs, RGB rhs)
  {
    return {min((lhs.R+rhs.R)/2,255),min((lhs.G+rhs.G)/2,255),min((lhs.B+rhs.B)/2,255)};
  }

  static RGB multiply(RGB color, double factor)
  {
    return {max(min(int(color.R*factor),255),0),max(min(int(color.G*factor),255),0),max(min(int(color.B*factor),255),0)};
  };

  static XYZ RGBtoXYZ(RGB input)
  {
    double r=inverseRGBCompanding(input.R/255.);
    double g=inverseRGBCompanding(input.G/255.);
    double b=inverseRGBCompanding(input.B/255.);

    double X=0.4124564*r+0.3575761*g+0.1804375*b;
    double Y=0.2126729*r+0.7151522*g+0.0721750*b;
    double Z=0.0193339*r+0.1191920*g+0.9503041*b;

    return {X,Y,Z};
  };

  static Lab XYZtoLab(XYZ input)
  {
    double X_R=0.9505;
    double Y_R=1.0000;
    double Z_R=1.0890;

    double epsilon=216.0/24389.0;
    double kappa=24389.0/27.0;

    double x_r=input.X/X_R;
    double y_r=input.Y/Y_R;
    double z_r=input.Z/Z_R;

    double f_x=(x_r>epsilon ? pow(x_r,1.0/3.0) : (kappa*x_r+16.0)/(116.0));
    double f_y=(y_r>epsilon ? pow(y_r,1.0/3.0) : (kappa*y_r+16.0)/(116.0));
    double f_z=(z_r>epsilon ? pow(z_r,1.0/3.0) : (kappa*z_r+16.0)/(116.0));

    double L=116*f_y-16.0;
    double a=500.0*(f_x-f_y);
    double b=200.0*(f_y-f_z);

    return {L,a,b};
  };

  static LCH LabtoLCH(Lab input)
  {
    double L=input.L;
    double C=sqrt(pow(input.a,2)+pow(input.b,2));

    double atanba=atan2(input.b,input.a);
    double H=(atanba>=0 ? atanba : atanba+2*M_PI);

    return {L,C,H};
  };

  static Lab LCHtoLab(LCH input)
  {
    double L=input.L;
    double a=input.C*cos(input.H);
    double b=input.C*sin(input.H);

    return {L,a,b};
  };

  static XYZ LabtoXYZ(Lab input)
  {
    double X_R=0.9505;
    double Y_R=1.0000;
    double Z_R=1.0890;

    double epsilon=216.0/24389.0;
    double kappa=24389.0/27.0;

    double f_y=(input.L+16.0)/116.0;
    double f_x=input.a/500.0+f_y;
    double f_z=f_y-input.b/200.0;

    double powf_x3=pow(f_x,3);
    double powf_z3=pow(f_z,3);

    double x_r=(powf_x3>epsilon ? powf_x3 : (116.0*f_x-16.0)/kappa);
    double y_r=(input.L>kappa*epsilon ? pow(f_y,3) : input.L/kappa);
    double z_r=(powf_z3>epsilon ? powf_z3 : (116.0*f_z-16.0)/kappa);

    double X=x_r*X_R;
    double Y=y_r*Y_R;
    double Z=z_r*Z_R;

    return {X,Y,Z};
  };

  static RGB XYZtoRGB(XYZ input)
  {
    double r=3.2404542*input.X-1.5371385*input.Y-0.4985314*input.Z;
    double g=-0.9692660*input.X+1.8760108*input.Y+0.0415560*input.Z;
    double b=0.0556434*input.X-0.2040259*input.Y+1.0572252*input.Z;

    r=max(min(r,1.0),0.0);
    g=max(min(g,1.0),0.0);
    b=max(min(b,1.0),0.0);

    double R=sRGBCompanding(r)*255.0;
    double G=sRGBCompanding(g)*255.0;
    double B=sRGBCompanding(b)*255.0;

    return {uint8_t(R),uint8_t(G),uint8_t(B)};
  };

  static LCH RGBtoLCH(RGB input)
  {
    return LabtoLCH(XYZtoLab(RGBtoXYZ(input)));
  };

  static RGB LCHtoRGB(LCH input)
  {
    return XYZtoRGB(LabtoXYZ(LCHtoLab(input)));
  };

  static double inverseRGBCompanding(double value)
  {
    if(value<=0.04045) {
      return value/12.92;
    } else {
      return pow((value+0.055)/1.055,2.4);
    }
  };

  static double sRGBCompanding(double value)
  {
    if(value<=0.0031308) {
      return 12.92*value;
    } else {
      return 1.055*pow(value,1.0/2.4)-0.055;
    }
  };

private:

  ColorHelper();

};

// explicit specialization for template
template <>
inline ColorHelper::LCH ColorHelper::Scale<ColorHelper::LCH>::getInterpolatedProperty(double value, double lowerBoundValue, ColorHelper::LCH lowerBoundProperty, double upperBoundValue, ColorHelper::LCH upperBoundProperty)
{
  ColorHelper::LCH lowerLCH=lowerBoundProperty;
  ColorHelper::LCH upperLCH=upperBoundProperty;

  ColorHelper::LCH interpolatedLCH={getInterpolatedPropertyDouble(value, lowerBoundValue, lowerLCH.L, upperBoundValue, upperLCH.L),
                                    getInterpolatedPropertyDouble(value, lowerBoundValue, lowerLCH.C, upperBoundValue, upperLCH.C),
                                    getInterpolatedPropertyHue(value, lowerBoundValue, lowerLCH.H, upperBoundValue, upperLCH.H)};

  return interpolatedLCH;
};

template <>
inline ColorHelper::RGB ColorHelper::Scale<ColorHelper::RGB>::getInterpolatedProperty(double value, double lowerBoundValue, ColorHelper::RGB lowerBoundProperty, double upperBoundValue, ColorHelper::RGB upperBoundProperty)
{
  ColorHelper::RGB interpolatedRGB={getInterpolatedPropertyDouble(value, lowerBoundValue, lowerBoundProperty.R, upperBoundValue, upperBoundProperty.R),
                                    getInterpolatedPropertyDouble(value, lowerBoundValue, lowerBoundProperty.G, upperBoundValue, upperBoundProperty.G),
                                    getInterpolatedPropertyDouble(value, lowerBoundValue, lowerBoundProperty.B, upperBoundValue, upperBoundProperty.B)};

  return interpolatedRGB;
};

#endif
