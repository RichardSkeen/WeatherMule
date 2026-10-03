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
#include "HttpClienus.h"

const String Url_CheckIn = "/api/checkin";
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

String UserAgentString();

//----------------------------------------------
// Implementation below here
//---------------------------------------------- 

bool CheckInApi()
{
    HttpResponsum response;
    String useToken = tokenRefresh;

    if(authAttempts >= max_AuthAttempts)
    {
        LogInformation("Max authentication attempts exceeded: " + String(max_AuthAttempts));

        return false;
    }

    // First boot or refresh token expired
    if (useToken.length() == 0)
    {
        LogInformation("Using API Token (" + String(authAttempts) + ")");

        useToken = PackStationWeather_API_KEY;
    }
    else {
        LogInformation("Using Refresh Token (" + String(authAttempts) + ")");
    }

    LogInformation("");

    response = HttpRequest(PackStationWeather_Host, PackStationWeather_Port, "GET", Url_CheckIn, useToken, UserAgentString(), "");

    authAttempts++;

    if(response.Succeeded != true || (response.StatusCode != 200 && response.StatusCode != 401))
    {
        //Http request failed not Auth related.
        LogInformation(response.StatusLine, response.ErrorMessage);

        return false;
    }
    else if(response.StatusCode == 401 && useToken == PackStationWeather_API_KEY)
    {
        //API key revoked
        authAttempts = max_AuthAttempts + 1;

        LogInformation("Api Key revoked");
        LogInformation();

        return false;
    }
    else if(response.StatusCode == 401)
    {
        //refresh token expired or revoked
        tokenRefresh = "";

        LogInformation("Refresh Token expired or revoked.");
        LogInformation();

        return CheckInApi();
    }

    // reponse.StatusCode = 200

    ApiResponse apiResponse(response.Body);

    if (apiResponse.IsValid != true)
    {
        LogInformation("CheckInApi: invalid JSON response");

        return false;
    }

    tokenSession = apiResponse.GetValue(0, "sessionToken");

    tokenRefresh = apiResponse.GetValue(0, "refreshToken");

    if (tokenSession.length() == 0 || tokenRefresh.length() == 0)
    {
        LogInformation("CheckInApi: token response incomplete");
        LogInformation();

        tokenSession = "";
        tokenRefresh = "";

        return false;
    }

    LogInformation("CheckInApi: tokens received");
    LogInformation();

    authAttempts = 0;

    return true;
}


bool UploadApi(WeatherRequest weatherRequest)
{
    ClearHoles();
    authAttempts = 0;

    HttpResponsum response = HttpRequest(PackStationWeather_Host, PackStationWeather_Port, "POST", Url_Upload, tokenSession, UserAgentString(), weatherRequest.ToJson());

    if(response.Succeeded != true || (response.StatusCode != 200 && response.StatusCode != 401))
    {
        LogInformation(response.StatusLine, response.ErrorMessage);

        return false;
    }
    else if(response.StatusCode == 401)
    {
        LogInformation("Authentication failed check in");

        if(CheckInApi() != true)
        {
            LogInformation("Check in failed.");

            return false;
        }

        return UploadApi(weatherRequest);
    }

    ApiResponse apiResponse(response.Body);

    if (apiResponse.IsValid != true)
    {
        LogInformation("UploadApi: invalid JSON response");

        return false;
    }

    AddHoles(apiResponse);

    return true;
}

bool FillHoleApi(WeatherRequest weatherRequest)
{
    LogInformation("Filling-----");
    LogInformation(weatherRequest.ToJson());

    HttpResponsum response = HttpRequest(PackStationWeather_Host, PackStationWeather_Port, "POST", Url_Fill, tokenSession, UserAgentString(), weatherRequest.ToJson());

    if(response.Succeeded != true || (response.StatusCode != 200 && response.StatusCode != 401))
    {
        LogInformation(response.StatusLine, response.ErrorMessage);

        LogInformation("Issues clearing holes");

        ClearHoles();

        return false;
    }
    else if(response.StatusCode == 401)
    {
        LogInformation("Authentication failed check in");

        if(CheckInApi() != true)
        {
            LogInformation("Check in failed.");

            return false;
        }

        return FillHoleApi(weatherRequest);
    }

    return true;
}

bool NotObservedApi(Hole* hole)
{
     LogInformation("NO: " + hole->ToJson());

    HttpResponsum response = HttpRequest(PackStationWeather_Host, PackStationWeather_Port, "POST", Url_NotObserved, tokenSession, UserAgentString(), hole->ToJson());

    if(response.Succeeded != true || (response.StatusCode != 200 && response.StatusCode != 401))
    {
        LogInformation(response.StatusLine, response.ErrorMessage);

        LogInformation("Issues clearing holes");
        ClearHoles();

        return false;
    }
    else if(response.StatusCode == 401)
    {
        LogInformation("Authentication failed check in");

        if(CheckInApi() != true)
        {
            LogInformation("Check in failed.");

            return false;
        }

        return NotObservedApi(hole);
    }

    return true;
}

String UserAgentString()
{
    return "WeatherMule/" +  String(WeatherMule_Name) + " (" + String(WeatherMule_Version) + ")";
}

