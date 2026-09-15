#pragma once

#include "Version.h"
#include <Arduino.h>
#include <vector>
#include "weathermule-secrets.h"
#include "KeyValuePair.h"
#include "WeatherRequest.h"
#include "StorageRoutines.h"
#include "ApiResponse.h"
#include "SerialMonitorRoutines.h"
#include "Hole.h"
#include <WiFiNINA.h>
#include <WiFiUdp.h>

const String Url_Upload = "/api/unload";
const String Url_Fill = "/api/unload/hole/fill";
const String Url_NotObserved = "/api/unload/hole/not/observed";

//----------------------------------------------
// Hole routine forward declarations
//----------------------------------------------
void AddHoles(const ApiResponse& apiResponse);
void ClearHoles();

const int max_AuthAttempts = 3;
int authAttempts = 0;

String tokenSession = "";
String tokenRefresh = "";

//----------------------------------------------
// Prototypes
//----------------------------------------------
bool CheckInApi();

bool UploadApi(WeatherRequest weatherRequest);
bool FillHoleApi(WeatherRequest weatherRequest);
bool NotObservedApi(Hole* hole);

String BuildHeaderAuth(
    const String& token);

String BuildUserAgent();

//----------------------------------------------
// Implementation below here
//---------------------------------------------- 
bool PostApi(const String& url, const String& jsonPayload, String& responseBody)
{
    responseBody = "";

    if(tokenSession.length() == 0)
    {
        if(!CheckInApi())
        {
            return false;
        }
    }

    WiFiClient apiClient;

    //Open Connection
    if (!apiClient.connect(
            PackStationWeather_Host,
            PackStationWeather_Port))
    {
        LogInformation("PostApi: connection failed");

        return false;
    }

    //Post
    apiClient.print("POST ");
    apiClient.print(url);
    apiClient.println(" HTTP/1.1");
    apiClient.println("Host: " PackStationWeather_Host);
    apiClient.println(BuildHeaderAuth(tokenSession));
    apiClient.println(BuildUserAgent());
    apiClient.println("Connection: close");
    
    //add payload if there is one
    if(jsonPayload.length() > 0)
    {
        apiClient.println("Content-Type: application/json");
        apiClient.println(
            "Content-Length: " +
            String(jsonPayload.length())
        );
        apiClient.println();
        apiClient.println(jsonPayload);
    }
    else{
        apiClient.println();
    }

    String statusLine = apiClient.readStringUntil('\n');

    LogInformation(statusLine);
    LogInformation("");

    if (statusLine.indexOf("401") >= 0)
    {
        authAttempts++;

        if(authAttempts > max_AuthAttempts)
        {
            return false;
        }

        if(!CheckInApi())
        {
            return false;
        }

        return PostApi(url, jsonPayload, responseBody);
    }

    //--------------------------------------
    // Skip headers
    //--------------------------------------

    while (apiClient.connected())
    {
        String line = apiClient.readStringUntil('\n');

        LogInformation(line);

        if (line == "\r")
        {
            break;
        }
    }

    LogInformation();
    //--------------------------------------
    // Read JSON body
    //--------------------------------------

    while (apiClient.available())
    {
        responseBody += (char)apiClient.read();
    }

    LogInformation("Response Body");
    LogInformation(responseBody);
    LogInformation();

    apiClient.stop();

    return true;
}

bool CheckInApi()
{
    String useToken = tokenRefresh;

    if(authAttempts > max_AuthAttempts)
    {
        LogInformation("Max authentication attempts exceeded: " + String(max_AuthAttempts));

        return false;
    }

    // First boot or refresh token expired
    if (useToken.length() == 0)
    {
        useToken = PackStationWeather_API_KEY;

        LogInformation("Using API Token");
    }
    else {
        LogInformation("Using Refresh Token");
    }

    LogInformation("");

    WiFiClient apiClient;

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
    apiClient.println(BuildUserAgent());
    apiClient.println("Connection: close");
    apiClient.println();

    //--------------------------------------
    // Read status line
    //--------------------------------------

    String statusLine = apiClient.readStringUntil('\n');

    LogInformation(statusLine);
    LogInformation("");

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

        authAttempts++;

        return false;
    }

    //--------------------------------------
    // Skip headers
    //--------------------------------------

    while (apiClient.connected())
    {
        String line = apiClient.readStringUntil('\n');

        LogInformation(line);

        if (line == "\r")
        {
            break;
        }
    }

    LogInformation("");
    //--------------------------------------
    // Read JSON body
    //--------------------------------------

    String responseBody = "";

    while (apiClient.available())
    {
        responseBody += (char)apiClient.read();
    }

    LogInformation("Response Body");
    LogInformation(responseBody);
    LogInformation("");

    apiClient.stop();

    ApiResponse response(responseBody);

    if (!response.IsValid)
    {
        LogInformation(
            "CheckInApi: invalid JSON response");

        return false;
    }

    tokenSession =
        response.GetValue(
            0,
            "sessionToken");

    tokenRefresh =
        response.GetValue(
            0,
            "refreshToken");

    if (tokenSession.length() == 0 ||
        tokenRefresh.length() == 0)
    {
        LogInformation(
            "CheckInApi: token response incomplete");

        tokenSession = "";
        tokenRefresh = "";

        return false;
    }

    LogInformation("CheckInApi: tokens received");

    authAttempts = 0;

    return true;
}


bool UploadApi(WeatherRequest weatherRequest)
{
    ClearHoles();

    String responseBody = "";

    bool postStatus = PostApi(Url_Upload, weatherRequest.ToJson(), responseBody);

    if(!postStatus)
    {
        return false;
    }

    ApiResponse response(responseBody);

    if (!response.IsValid)
    {
        LogInformation(
            "UploadApi: invalid JSON response");

        return false;
    }

    AddHoles(response);

    return true;
}

bool FillHoleApi(WeatherRequest weatherRequest)
{
    String responseBody;

    return PostApi(
        Url_Fill,
        weatherRequest.ToJson(),
        responseBody);
}

bool NotObservedApi(Hole* hole)
{
    String responseBody;

    return PostApi(
        Url_NotObserved,
        hole->ToJson(),
        responseBody);
}

String BuildHeaderAuth(const String& token)
{
    return "Authorization: Bearer " + token;
}

String BuildUserAgent()
{
    return "User-Agent: WeatherMule/" +
           String(WeatherMule_Name) +
           " (" +
           String(WeatherMule_Version) +
           ")";
}