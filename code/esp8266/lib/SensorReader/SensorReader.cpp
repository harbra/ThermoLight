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

#include "SensorReader.h"

// class definitions
SensorReader::SensorReader() : SHT31_measduration(30), SHT31_updateint(5000), SHT31_lastmillis(0), SHT31_waitmeasurement(false), lastTemp(NAN), lastRH(NAN), SHT31_TempOffset(0), millis_lastValuesTempRH(0), CCS811_found(false), CCS811_updateint(10000), CCS811_lastmillis(0), lasteCO2(NAN), lastTVOC(NAN), millis_lastValueseCO2TVOC(0)
{
}

bool SensorReader::setup(uint8_t i2cSHT31, uint8_t i2cCCS811)
{
  i2c_SHT31=i2cSHT31;
  i2c_CCS811=i2cCCS811;

  Wire.begin();

  #ifdef ESP8266
    Wire.setClockStretchLimit(500);
  #endif

  reset();
  delay(100);

  writeCommand(i2c_SHT31, SHT31_Commands::SHT31_HEATERDIS);

  CCS811_found=(read(i2c_CCS811,CCS811_Commands::HW_ID)==CCS811_HW_ID);
  CCS811_found&=getStatus().APP_VALID;

  writeCommand(i2c_CCS811,CCS811_Commands::APP_START, nullptr, 0);

  delay(100);

  CCS811_Status status=getStatus();
  CCS811_found&=!status.ERROR;
  CCS811_found&=status.FW_MODE;

  if(CCS811_found) {
    CCS811_measmode.DRIVE_MODE=CCS811_Modes::Pulsed_10s;
    uint8_t temp=CCS811_measmode.get();
    writeCommand(i2c_CCS811,CCS811_Commands::MEAS_MODE,&temp,1);
  }

  return CCS811_found;
}

void SensorReader::attachHandlers(ValueReceivedCallback onNewTemperature, ValueReceivedCallback onNewRelativeHumidity, ValueReceivedCallback onNewequivalentCO2, ValueReceivedCallback onNewTotalVOC)
{
  onNewTemp=onNewTemperature;
  onNewRH=onNewRelativeHumidity;
  onNeweCO2=onNewequivalentCO2;
  onNewTVOC=onNewTotalVOC;
}

void SensorReader::handle()
{
  if(CCS811_found) {
    if(millis()>CCS811_lastmillis+CCS811_updateint) {
      uint8_t buf[5];

      CCS811_Status status=getStatus();

      if(status.DATA_READY) {
        read(i2c_CCS811,CCS811_Commands::ALG_RESULT_DATA,buf,4);

        lasteCO2=(buf[0]<<8) | (buf[1]);
        lastTVOC=(buf[2]<<8) | (buf[3]);
        millis_lastValueseCO2TVOC=millis();

        if(onNeweCO2 && !std::isnan(lasteCO2)) {
          onNeweCO2(lasteCO2);
        }

        if(onNewTVOC && !std::isnan(lastTVOC)) {
          onNewTVOC(lastTVOC);
        }
      }

      CCS811_lastmillis=millis();
    }
  }
  if(SHT31_waitmeasurement) {
    if(millis()>SHT31_lastmillis+SHT31_measduration) {
      Wire.requestFrom(i2c_SHT31,uint8_t(6));

      uint8_t buffer[6];
      for(uint8_t i=0; i<6; i++) {
        buffer[i]=Wire.read();
      }

      uint16_t temp, rh;

      temp=(buffer[0]<<8) | buffer[1];
      rh=(buffer[3]<<8) | buffer[4];

      if(buffer[2]==crc8(buffer,2)) {
        lastTemp=-45.+175.*double(temp)/double(0xFFFF)+SHT31_TempOffset; // the "official" formula uses -45
      } else {
        myLogger.addToLog("CRC Error Temp: "+String(buffer[0])+" "+String(buffer[1])+" "+String(buffer[2])+" ");
        lastTemp=NAN;
      }

      if(buffer[5]==crc8(buffer+3,2)) {
        lastRH=100.*double(rh)/double(0xFFFF);
      } else {
        myLogger.addToLog("CRC Error RH: "+String(buffer[3])+" "+String(buffer[4])+" "+String(buffer[5])+" ");
        lastRH=NAN;
      }

      millis_lastValuesTempRH=millis();

      if(CCS811_found) {
        setCCS811Environment(lastTemp,lastRH);
      }

      if(onNewTemp && !std::isnan(lastTemp)) {
        onNewTemp(lastTemp);
      }

      if(onNewRH && !std::isnan(lastTemp)) {
        onNewRH(lastRH);
      }


      SHT31_waitmeasurement=false;
    }
  } else {
    if(millis()>SHT31_lastmillis+SHT31_updateint) {
      writeCommand(i2c_SHT31, SHT31_Modes::SHT31_MEAS_HIGHREP);

      SHT31_waitmeasurement=true;
      while(millis()>SHT31_lastmillis+SHT31_updateint) {
        SHT31_lastmillis+=SHT31_updateint;
      }
    }
  }
}

void SensorReader::setCCS811Environment(double temp, double hum)
{
  if(std::isnan(temp) || std::isnan(hum)) {
    return;
  }

  uint8_t hum_high=uint8_t(hum)<<1;
  uint8_t hum_low=0;

  float frac=modf(temp,&temp);
  uint16_t temp_high=(uint16_t(temp)+25)<<9;
  uint16_t temp_low=uint16_t(frac/0.001953125) & 0x1FF;
  uint16_t temp_16=temp_high | temp_low;

  uint8_t buf[4]={hum_high, hum_low, uint8_t((temp_16 >> 8) & 0xFF), uint8_t(temp_16 & 0xFF)};

  writeCommand(i2c_CCS811, CCS811_Commands::ENV_DATA, buf, 4);
}

void SensorReader::reset()
{
  writeCommand(i2c_SHT31, SHT31_Commands::SHT31_SOFTRESET);

  uint8_t seq[] = {0x11, 0xE5, 0x72, 0x8A};
  writeCommand(i2c_CCS811, CCS811_Commands::SW_RESET, seq, 4);
}

SensorReader::CCS811_Status SensorReader::getStatus()
{
  CCS811_Status out;
  uint8_t ret;
  read(i2c_CCS811, CCS811_Commands::STATUS, &ret, 1);
  out.set(ret);

  return out;
}

void SensorReader::writeCommand(uint8_t address, uint16_t command)
{
  Wire.beginTransmission(address);
  Wire.write(command >> 8);
  Wire.write(command & 0xFF);
  Wire.endTransmission();
}

void SensorReader::writeCommand(uint8_t address, uint8_t reg, uint8_t* buf, uint8_t num)
{
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(buf, num);
  Wire.endTransmission();
}

uint8_t SensorReader::read(uint8_t address, uint8_t reg)
{
  uint8_t ret;
  read(address, reg, &ret, 1);

  return ret;
}

void SensorReader::read(uint8_t address, uint8_t reg, uint8_t* buf, uint8_t num)
{
  uint8_t pos = 0;

  //on arduino we need to read in 32 byte chunks
  while(pos < num){

    uint8_t read_now = min((uint8_t)32, (uint8_t)(num - pos));
    Wire.beginTransmission((uint8_t)address);
    Wire.write((uint8_t)reg + pos);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)address, read_now);

    for(int i=0; i<read_now; i++){
      buf[pos] = Wire.read();
      pos++;
    }
  }
}

uint8_t SensorReader::crc8(const uint8_t *data, int len)
{
/*
*
 * CRC-8 formula from page 14 of SHT spec pdf
 *
 * Test data 0xBE, 0xEF should yield 0x92
 *
 * Initialization data 0xFF
 * Polynomial 0x31 (x8 + x5 +x4 +1)
 * Final XOR 0x00
 */

  const uint8_t POLYNOMIAL(0x31);
  uint8_t crc(0xFF);

  for ( int j = len; j; --j ) {
      crc ^= *data++;

      for ( int i = 8; i; --i ) {
	crc = ( crc & 0x80 )
	  ? (crc << 1) ^ POLYNOMIAL
	  : (crc << 1);
      }
  }
  return crc;
}
