#pragma once

#include "WeatherParam.h"
#include "WeatherRequest.h"
#include "StorageRoutines.h"
#include "ApiClientRoutines.h"
#include "Hole.h"
#include <WiFiNINA.h>
#include <WiFiUdp.h>

extern bool inDesignMode;

//----------------------------------------------
// Prototypes
//---------------------------------------------- 

void ProcessWeatherStationRequest(WiFiClient& client);
void SendStatusResponse(
    WiFiClient& client,
    int statusCode,
    const String& statusText);

void Send200(WiFiClient& client);
void Send400(WiFiClient& client);
void Send404(WiFiClient& client);
void Send500(WiFiClient& client);
void Send503(WiFiClient& client);
void Send507(WiFiClient& client);
void SendDesignModeResponse(WiFiClient& client, bool designMode);

//----------------------------------------------
// Implementation below here
//---------------------------------------------- 

void ProcessWeatherStationRequest(WiFiClient& client)
{
  String request = client.readStringUntil('\r');
  
  WeatherRequest weatherRequest(request);

  if(weatherRequest.isWeatherRequest){
    

    if(AppendToBox(weatherRequest))
    {
        //UploadWeatherStationRequest(weatherRequest);

        Send200(client);
    }
    else
    {
        Send500(client);
    }


    if(inDesignMode){
      LogInformation(weatherRequest);
    }
  }
  else if(request.indexOf("/logging/on") > -1){
    inDesignMode = true;
    SendDesignModeResponse(client, inDesignMode);
  }
  else if(request.indexOf("/logging/off") > -1){
    inDesignMode = false;
    SendDesignModeResponse(client, inDesignMode);
  }
  else{
    Send404(client);
  }

}

void SendStatusResponse(
    WiFiClient& client,
    int statusCode,
    const String& statusText)
{
    client.println("HTTP/1.1 " + String(statusCode) + " " + statusText);
    client.println("Connection: close"); 
    client.println(); // IMPORTANT: blank line separates headers from body
}

void Send200(WiFiClient& client)
{
    SendStatusResponse(client, 200, "OK");
}

void Send400(WiFiClient& client)
{
    SendStatusResponse(client, 400, "Bad Request");
}

void Send500(WiFiClient& client)
{
    SendStatusResponse(client, 500, "Internal Server Error");
}

void Send503(WiFiClient& client)
{
    SendStatusResponse(client, 503, "Service Unavailable");
}

void Send507(WiFiClient& client)
{
    SendStatusResponse(client, 507, "Insufficient Storage");
}

void Send404(WiFiClient& client)
{
    SendStatusResponse(client, 404, "Not Found");
}

void SendDesignModeResponse(WiFiClient& client, bool designMode){
  SendStatusResponse(client, 200, "OK");

  client.println("<!DOCTYPE html>");
  client.println("<html>");
  client.println("<head>");
  client.println("<title>Weather Mule</title>");
  client.println("</head>");
  client.println("<body>");
  client.println("<p>Design Mode:" + String(designMode == true ? "true" : "false") + "</p>");
  client.println("</body>");
  client.println("</html>");
}