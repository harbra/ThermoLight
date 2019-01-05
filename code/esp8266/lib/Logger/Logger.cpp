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

#include "Logger.h"

Logger::Logger() : m_forwardCallback(nullptr)
{
}

void Logger::addToLog(const String& message, LoggerVerbosity verbosity)
{
  String totalmessage=String(millis());
  totalmessage+=": ";
  totalmessage+=message;
  Serial.println(totalmessage);

  if(m_forwardCallback!=nullptr) m_forwardCallback(totalmessage);
}

void Logger::addToLogLineStart(const String& message, LoggerVerbosity verbosity)
{
  String totalmessage=String(millis());
  totalmessage+=": ";
  totalmessage+=message;
  Serial.print(totalmessage);

  if(m_forwardCallback!=nullptr) m_forwardCallback(totalmessage);
}

void Logger::addToLogLineAdd(const String& message, LoggerVerbosity verbosity)
{
  Serial.print(message);

  if(m_forwardCallback!=nullptr) m_forwardCallback(message);
}

void Logger::addToLogLineEnd(const String& message, LoggerVerbosity verbosity)
{
  Serial.println(message);

  if(m_forwardCallback!=nullptr) m_forwardCallback(message);
}

void Logger::addToLog(const String& message1, const String& message2, LoggerVerbosity verbosity)
{
  String totalmessage=String(millis());
  totalmessage+=": ";
  totalmessage+=message1;
  totalmessage+=message2;
  Serial.println(totalmessage);

  if(m_forwardCallback!=nullptr) m_forwardCallback(totalmessage);
}

void Logger::enableSerialForward(SerialEventCallback forwardCallback)
{
  m_forwardCallback=forwardCallback;
}

void Logger::disableSerialForward() {
  m_forwardCallback=nullptr;
}

bool Logger::isSerialEnabled() {
  return !(m_forwardCallback==nullptr);
}

Logger myLogger=Logger();
