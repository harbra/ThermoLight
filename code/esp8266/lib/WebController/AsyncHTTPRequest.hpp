/* AsyncHTTPRequest.hpp

Purpose
=======
AsyncHTTPRequest.hpp is used as container for ESPAsyncTCP and used for HTTP requests.
The body of the response is kept and a callback is called once the response has been received.

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

#ifndef AsyncHTTPRequest_h
#define AsyncHTTPRequest_h

#include <ESPAsyncTCP.h>

#include <Logger.h>

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

typedef std::function<void(int status, char* body)> RequestFinishedCallback;

class AsyncHTTPRequest
{
public:

  AsyncHTTPRequest(String url, int port) : m_client(nullptr), m_finishedCallback(nullptr), m_url(url), m_port(port), m_dataposition(0), m_databuffer(nullptr), m_stringbuffer(""), m_bodyreached(false), m_status(-1), m_deletebuffer(true) {
    if(url.startsWith("http://")) {
      int delimiterpos=url.indexOf("/",7);
      m_host=url.substring(7,delimiterpos);
      m_path=url.substring(delimiterpos);
    } else {
      int delimiterpos=url.indexOf("/");
      m_host=url.substring(0,delimiterpos);
      m_path=url.substring(delimiterpos);
    }
  };

  ~AsyncHTTPRequest() {
    if(m_databuffer!=nullptr) {
      delete[] m_databuffer;
      m_databuffer=nullptr;
    }

    if(m_client!=nullptr) {
      delete m_client;
      m_client=nullptr;
    }
  }

  String getUrl() {
    return m_url;
  }

  int getPort() {
    return m_port;
  }

  bool send(RequestFinishedCallback finishedCallback, bool deletebuffer=true) {
    m_finishedCallback=finishedCallback;
    m_deletebuffer=deletebuffer;

    if(m_client) {
      myLogger.addToLog("Client already exists");
      return false;
    }

    m_client=new AsyncClient();
    if(!m_client) {
      myLogger.addToLog("Couldn't create client");
      return false;
    }

    m_client->onError(handleError, this);
    m_client->onConnect(handleConnect, this);

    m_status=-1;
    m_dataposition=0;
    if(m_databuffer!=nullptr) {
      delete[] m_databuffer;
      m_databuffer=nullptr;
    }

    if(!m_client->connect(m_host.c_str(),m_port)) {
      myLogger.addToLog("Connection failed");
      AsyncClient * client=m_client;
      m_client=nullptr;
      delete client;

      return false;
    }

    return true;
  };

  void clearBuffer() {
    if(m_databuffer!=nullptr) {
      delete[] m_databuffer;
      m_databuffer=nullptr;
    }
  };

private:

  AsyncClient* m_client;
  RequestFinishedCallback m_finishedCallback;

  String m_url;
  String m_host;
  String m_path;
  int m_port;

  int m_dataposition;
  char* m_databuffer;
  String m_stringbuffer;
  bool m_bodyreached;
  int m_status;
  bool m_deletebuffer;

  static void handleConnect(void * arg, AsyncClient * c) {
    AsyncHTTPRequest* self=(AsyncHTTPRequest*)arg;

    self->m_client->onError(NULL,NULL);

    self->m_client->onDisconnect(handleDisconnect,arg);
    self->m_client->onData(handleData,arg);

    self->m_client->write(("GET "+self->m_path+" HTTP/1.1\r\nHost: "+self->m_host+"\r\nConnection: close\r\n\r\n").c_str());
  };

  static void handleDisconnect(void * arg, AsyncClient * c) {
    AsyncHTTPRequest* self=(AsyncHTTPRequest*)arg;

    //myLogger.addToLog("Body: "+String(self->m_databuffer));

    if(self->m_finishedCallback) {
      self->m_finishedCallback(self->m_status, self->m_databuffer);
    }

    if(self->m_deletebuffer) {
      delete[] self->m_databuffer;
      self->m_databuffer=nullptr;
    }
    self->m_dataposition=0;
    self->m_status=-1;
    self->m_stringbuffer="";
    self->m_bodyreached=false;

    self->m_client=nullptr;
    delete c;
  };

  static void handleData(void * arg, AsyncClient * c, void * data, size_t len) {
    AsyncHTTPRequest* self=(AsyncHTTPRequest*)arg;

    char* d = (char*)data;

    int startpos=0;
    if(self->m_bodyreached==false) {
      // didn't reach body yet
      int bufferlength=0;

      for(int i=startpos; i<len; i++) {
        if(d[i]=='\n') {
          // found end of line
          char line[i-startpos+1];
          line[i-startpos]='\0';
          strncpy(line,d+startpos,i-startpos);
          if(startpos==0) {
            // first response line should contain HTTP status code
            String statusLine=self->m_stringbuffer+String(line);
            statusLine=statusLine.substring(statusLine.indexOf(" ")+1);
            statusLine=statusLine.substring(0,statusLine.indexOf(" "));
            //myLogger.addToLog("Status: "+statusLine);

            self->m_status=statusLine.toInt();
          } else {
            String strLine=self->m_stringbuffer+String(line);
            if(strLine.substring(0,15).equalsIgnoreCase("Content-Length:")) {
              bufferlength=strLine.substring(15).toInt();
              //myLogger.addToLog("Length: "+String(bufferlength));
              self->m_databuffer=new char[bufferlength+1];
              self->m_databuffer[bufferlength]='\0';
            }
          }
          startpos=i+1;

          if(i+2<len && d[i+2]=='\n') {
              if(d[i-1]=='\r' && d[i+1]=='\r') {
                //myLogger.addToLog("Body found.");
                self->m_bodyreached=true;
                startpos=i+3;
                break;
              }
          }
        }
      }
      if(startpos<len && !self->m_bodyreached) {
        // there is unprocessed text remaining at the end of the chunk
        char line[len-startpos+1];
        line[len-startpos]='\0';
        strncpy(line,d+startpos,len-startpos);
        self->m_stringbuffer=String(line);
        startpos=len;
      }
    }

    if(self->m_bodyreached) {
      memcpy(self->m_databuffer+self->m_dataposition,d+startpos,len-startpos);
      self->m_dataposition+=len-startpos;
    }
  };

  static void handleError(void * arg, AsyncClient * c, int error) {
    AsyncHTTPRequest* self=(AsyncHTTPRequest*)arg;

    myLogger.addToLog("AsyncHTTPRequest: Error: "+String(self->m_client->errorToString(error)));

    self->m_status=-1;
    self->m_dataposition=0;
    self->m_stringbuffer="";
    self->m_bodyreached=false;
    if(self->m_databuffer!=nullptr) {
      delete[] self->m_databuffer;
      self->m_databuffer=nullptr;
    }

    self->m_client=nullptr;
    delete c;
  };

};

#endif
