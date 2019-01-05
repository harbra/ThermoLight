/* SensorReader.h

Purpose
=======
SensorReader.h is used to get sensor data from an SHT31-D and CCS881. Some of the code is based on the respective
Adafruit libraries, however it has been adapted so that calls are non blocking (e.g. no delay is used).

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

#ifndef SensorReader_h
#define SensorReader_h

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

#include <Wire.h>

#include <Logger.h>

typedef std::function<void(double value)> ValueReceivedCallback;

class SensorReader
{
public:

  #define SHT31_DEFAULT_ADDR 0x44

  #define CCS811_DEFAULT_ADDR 0x5A
  #define CCS811_HW_ID 0x81

  typedef enum {
    SHT31_MEAS_HIGHREP_STRETCH=0x2C06,
    SHT31_MEAS_MEDREP_STRETCH =0x2C0D,
    SHT31_MEAS_LOWREP_STRETCH =0x2C10,
    SHT31_MEAS_HIGHREP=0x2400,
    SHT31_MEAS_MEDREP=0x240B,
    SHT31_MEAS_LOWREP=0x2416,
  } SHT31_Modes;

  typedef enum {
    SHT31_READSTATUS=0xF32D,
    SHT31_CLEARSTATUS=0x3041,
    SHT31_SOFTRESET=0x30A2,
    SHT31_HEATEREN=0x306D,
    SHT31_HEATERDIS=0x3066
  } SHT31_Commands;

  typedef enum {
    STATUS=0x00,
    MEAS_MODE=0x01,
    ALG_RESULT_DATA=0x02,
    RAW_DATA=0x03,
    ENV_DATA=0x05,
    NTC=0x06,
    THRESHOLDS=0x10,
    BASELINE=0x11,
    HW_ID=0x20,
    HW_Version=0x21,
    FW_Boot_Version=0x23,
    FW_App_Version=0x24,
    Internal_State=0xA0,
    ERROR_ID=0xE0,
    APP_ERASE=0xF1,
    APP_DATA=0xF2,
    APP_VERIFY=0xF3,
    APP_START=0xF4,
    SW_RESET=0xFF
  } CCS811_Commands;

  typedef enum {
    Idle=0x00,
    Constant_1s=0x01,
    Pulsed_10s=0x02,
    Pulsed_60s=0x03,
    Constans_250ms=0x04
  } CCS811_Modes;

  typedef enum {
    WRITE_REG_INVALID=0x00,
    READ_REG_INVALID=0x01,
    MEASMODE_INVALID=0x02,
    MAX_RESISTANCE=0x03,
    HEATER_FAULT=0x04,
    HEATER_SUPPLY=0x05
  } CCS811_Error;

  struct CCS811_Status {
    uint8_t ERROR : 1;
    uint8_t : 2;
    uint8_t DATA_READY : 1;
    uint8_t APP_VALID : 1;
    uint8_t APP_VERIFY : 1;
    uint8_t APP_ERASE : 1;
    uint8_t FW_MODE : 1;

    void set(uint8_t data) {
      ERROR=data & 0x01;
      DATA_READY=(data >> 3) & 0x01;
      APP_VALID=(data >> 4) & 0x01;
      APP_VERIFY=(data >> 5) & 0x01;
      APP_ERASE=(data >> 6) & 0x01;
      FW_MODE=(data >> 7) & 0x01;
    };

    uint8_t get() {
      return (ERROR) | (DATA_READY << 3) | (APP_VALID << 4) | (APP_VERIFY << 5) | (APP_ERASE << 6) | (FW_MODE << 7);
    }
  };

  struct CCS811_MeasMode {
    uint8_t : 2;
    uint8_t THRESH : 1;
    uint8_t INTERRUPT : 1;
    uint8_t DRIVE_MODE : 3;
    uint8_t : 1;

    void set(uint8_t data) {
      THRESH=(data >> 2) & 0x01;
      INTERRUPT=(data >> 3) & 0x01;
      DRIVE_MODE=(data >> 4) & 0x01;
    };

    uint8_t get() {
      return (THRESH << 2) | (INTERRUPT << 3) | (DRIVE_MODE << 4);
    }
  };

  SensorReader();
  bool setup(uint8_t i2cSHT31 = SHT31_DEFAULT_ADDR, uint8_t i2cCCS811 = CCS811_DEFAULT_ADDR);
  void attachHandlers(ValueReceivedCallback onNewTemperature, ValueReceivedCallback onNewRelativeHumidity, ValueReceivedCallback onNewequivalentCO2, ValueReceivedCallback onNewTotalVOC);
  void handle();

  void reset();

  double getTemperature() { return lastTemp; };
  double getHumidity() { return lastRH; };
  double geteCO2() {return lasteCO2; };
  double getTVOC() {return lastTVOC; };

  unsigned int getMillisTH() { return millis_lastValuesTempRH; };
  unsigned int getMillisCT() { return millis_lastValueseCO2TVOC; };

private:

  void writeCommand(uint8_t address, uint16_t command);
  void writeCommand(uint8_t address, uint8_t reg, uint8_t* buf, uint8_t num);
  void read(uint8_t address, uint8_t reg, uint8_t* buf, uint8_t num);
  uint8_t read(uint8_t address, uint8_t reg);
  uint8_t crc8(const uint8_t *data, int len);

  void setCCS811Environment(double temp, double hum);

  uint8_t i2c_SHT31;
  unsigned long SHT31_measduration;
  unsigned long SHT31_updateint;
  unsigned long SHT31_lastmillis;
  bool SHT31_waitmeasurement;

  double lastTemp;
  double lastRH;
  double SHT31_TempOffset;
  unsigned long millis_lastValuesTempRH;
  ValueReceivedCallback onNewTemp;
  ValueReceivedCallback onNewRH;

  uint8_t i2c_CCS811;
  bool CCS811_found;
  unsigned long CCS811_updateint;
  unsigned long CCS811_lastmillis;

  double lasteCO2;
  double lastTVOC;
  unsigned long lastValuesmillis;
  unsigned long millis_lastValueseCO2TVOC;
  ValueReceivedCallback onNeweCO2;
  ValueReceivedCallback onNewTVOC;

  CCS811_MeasMode CCS811_measmode;
  CCS811_Status getStatus();

};

#endif
