#pragma once
#include "WeatherRequest.h"
#include "WeatherParam.h"

//----------------------------------------------
// Prototypes
//---------------------------------------------- 
void LogInformation();
void LogInformation(const String& logThis);
void LogInformation(WeatherRequest& logWeatherRequest);
String ToHexString(uint32_t value);

//----------------------------------------------
// Implementation below here
//---------------------------------------------- 

void LogInformation()
{
  Serial.println();
}

void LogInformation(const String& logThis)
{
  Serial.println(logThis);
}

void LogInformation(WeatherRequest& logWeatherRequest)
{
  Serial.println(logWeatherRequest.httpRequest);

  for (const WeatherParam& param : logWeatherRequest.Parameters) {
    Serial.print(param.Name);
    Serial.print(": ");
    Serial.println(param.Value);
  }

  Serial.println();
}

String ToHexString(uint32_t value)
{
    String result = String(value, HEX);
    result.toUpperCase();
    return "0x" + result;
}
