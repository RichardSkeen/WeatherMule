#include <vector>
#include "weathermule-secrets.h"
#include "WeatherStationRoutines.h"
#include "StorageRoutines.h"
#include "HoleRoutines.h"
#include "Hole.h"
#include "WiFiRoutines.h"
#include <WiFiNINA.h>
#include <WiFiUdp.h>

bool inDesignMode;
int loopCount = 0;

WiFiServer weatherStationServer(80);

bool giveFeedback = false;
bool designModeChanged = false;


void setup()
{
    Serial.begin(9600);

    while (!Serial)
    {
        ; // Wait for Serial Monitor
    }

    inDesignMode = true;

    LogInformation(
        "Version: " + String(WeatherMule_Version),
        true);

    //
    // Initialize storage independently of WiFi.
    //
    sdAvailable = InitializeStorage();

    //
    // MaintainWiFi performs the initial connection attempt.
    // If WiFi is unavailable it will retry every five minutes.
    //
    MaintainWiFi();

    delay(500);

    byte mac[6];
    WiFi.macAddress(mac);

    LogInformation(
        "Mac: " + MacToString(mac),
        true);

    LogInformation(true);
    LogInformation(true);
}

void loop()
{
    MaintainWiFi();

    if(!WiFiAvailable())
    {
        return;
    }

    WiFiClient incomingWebRequest =
        weatherStationServer.available();

    if(incomingWebRequest)
    {
        ProcessWeatherStationRequest(
            incomingWebRequest);

        delay(500);
        loopCount++;
    }
    else if(holeCount > 0)
    {
        ProcessHoles();
    }
}





String MacToString(byte mac[])
{
  String retMac = "";

  for(int i = 0; i < 6; i++)
  {
      if(i > 0)
      {
        retMac += ":";
      }

      retMac += ((mac[i]<16 ? "0" : "") + String(mac[i], HEX));
  }

  return retMac;
}
