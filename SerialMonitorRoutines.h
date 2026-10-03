#pragma once

#include "WeatherRequest.h"
#include "KeyValuePair.h"

extern bool inDesignMode;

//----------------------------------------------
// Prototypes
//---------------------------------------------- 
void LogInformation();
void LogInformation(bool force);
void LogInformation(const String& logThis);
void LogInformation(const String& logThis, bool force);
void LogInformation(const String& statusLine, const String& errorMessage);
void LogInformation(const String& statusLine, const String& errorMessage, bool force);
void LogInformation(WeatherRequest& logWeatherRequest);
String ToHexString(uint32_t value);

//----------------------------------------------
// Implementation below here
//---------------------------------------------- 

void LogInformation()
{
  LogInformation(false);
}

void LogInformation(bool force)
{
  if(inDesignMode || force)
  {
    Serial.println();
  }
}

void LogInformation(const String& logThis)
{
  LogInformation(logThis, false);
}

void LogInformation(const String& logThis, bool force)
{
  if(inDesignMode || force)
  {
    Serial.println(logThis);
  }
}

void LogInformation(const String& statusLine, const String& errorMessage)
{
  LogInformation(statusLine, errorMessage, false);
}

void LogInformation(const String& statusLine, const String& errorMessage, bool force)
{
  if(inDesignMode || force)
  {
    Serial.println("Http request failed");

    if(statusLine.length() > 0)
    {
        Serial.println(statusLine);
    }

    if(errorMessage.length() > 0)
    {
        Serial.println(errorMessage);
    }

    Serial.println();
  }
}

void LogInformation(WeatherRequest& logWeatherRequest)
{
  if(inDesignMode)
  {
    Serial.println(logWeatherRequest.httpRequest);

    for (const KeyValuePair& param : logWeatherRequest.Properties) {
      Serial.print(param.Name);
      Serial.print(": ");
      Serial.println(param.Value);
    }

    Serial.println();
  }
}

String ToHexString(uint32_t value)
{
    String result = String(value, HEX);
    result.toUpperCase();
    return "0x" + result;
}
