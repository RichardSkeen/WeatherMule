#pragma once

#include "weathermule-secrets.h"
#include "WeatherParam.h"
#include "WeatherRequest.h"
#include "StorageRoutines.h"
#include "Hole.h"
#include <WiFiNINA.h>
#include <WiFiUdp.h>

const int max_Holes = 10;

Hole Holes[max_Holes] ;
int holeCount;

String tokenSession = "";
String tokenRefresh = "";

//----------------------------------------------
// Prototypes
//---------------------------------------------- 
void UploadWeatherStationRequest(WeatherRequest weatherRequest);
bool CheckInApi();
void UplaodApi(const String& jsonObservation);
String BuildHeaderAuth(const String& token);

//----------------------------------------------
// Implementation below here
//---------------------------------------------- 
void UploadWeatherStationRequest(WeatherRequest weatherRequest)
{
   CheckInApi();

}

bool CheckInApi()
{
    String useToken = tokenRefresh;

    // First boot or refresh token expired
    if (useToken.length() == 0)
    {
        useToken = PackStationWeather_API_KEY;
    }

    WiFiClient apiClient;

        LogInformation("pre connection");
        LogInformation(PackStationWeather_Host);
        LogInformation(String(PackStationWeather_Port));

    if (!apiClient.connect(
            PackStationWeather_Host,
            PackStationWeather_Port))
    {
        LogInformation("CheckInApi: connection failed");

        return false;
    }

    apiClient.println("GET /api/checkin HTTP/1.1");
    apiClient.println("Host: " PackStationWeather_Host);
    apiClient.println(BuildHeaderAuth(useToken));
    apiClient.println("Connection: close");
    apiClient.println();

    //--------------------------------------
    // Read status line
    //--------------------------------------

    String statusLine = apiClient.readStringUntil('\n');

    LogInformation(statusLine);

    if (statusLine.indexOf("401") >= 0)
    {
        // refresh token rejected
        if (useToken != PackStationWeather_API_KEY)
        {
            apiClient.stop();

            LogInformation("trying API Key");

            tokenRefresh = "";
            return CheckInApi(); // retry with API key
        }

        LogInformation("CheckInApi: unauthorized");
        return false;
    }

    //--------------------------------------
    // Skip headers
    //--------------------------------------

    while (apiClient.connected())
    {
        String line = apiClient.readStringUntil('\n');

        LogInformation("line: " + line);

        if (line == "\r")
        {
            break;
        }
    }

    //--------------------------------------
    // Read JSON body
    //--------------------------------------

    String responseBody = "";

    while (apiClient.available())
    {
        responseBody += (char)apiClient.read();
    }

    apiClient.stop();

    //--------------------------------------
    // TODO:
    // parse JSON and populate:
    // tokenSession
    // tokenRefresh
    //--------------------------------------

    LogInformation(responseBody);

    return true;
}


void UplaodApi(const String& jsonObservation)
{
  if(tokenSession.length() == 0)
  {
    CheckInApi();
  }

  String headerAuth = BuildHeaderAuth(tokenSession);

}

String BuildHeaderAuth(const String& token)
{
    return "Authorization: Bearer " + token;
}
