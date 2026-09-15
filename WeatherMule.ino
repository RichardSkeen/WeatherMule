#include <vector>
#include "weathermule-secrets.h"
#include "WeatherStationRoutines.h"
#include "StorageRoutines.h"
#include "HoleRoutines.h"
#include "Hole.h"
#include <WiFiNINA.h>
#include <WiFiUdp.h>

bool inDesignMode;
int loopCount = 0;

WiFiServer server(80);

bool giveFeedback = false;
bool designModeChanged = false;


void setup() {
  Serial.begin(9600);

  inDesignMode = false;

  LogInformation("Connecting to WiFi...");
  
  while(WiFi.begin(SECRET_SSID, SECRET_PASS) != WL_CONNECTED)
  {
    Serial.print(".");

    delay(1000);
  }

  delay(1000);

  server.begin();

  sdAvailable = InitializeStorage();

  delay(5000);

  LogInformation("SSID: " + String(WiFi.SSID()), true);

  IPAddress ip = WiFi.localIP();
  LogInformation("IP: " + ip.toString(), true);

  byte mac[6];
  WiFi.macAddress(mac);
  LogInformation("Mac: "+ MacToString(mac), true);

  delay(500);

  LogInformation(true);
  LogInformation(true);
}

void loop() {

  WiFiClient incomingWebRequest = server.available();

  if(incomingWebRequest)
  { 
      ProcessWeatherStationRequest(incomingWebRequest);

      delay(500);

      incomingWebRequest.stop();

      loopCount++;
  }
  else if(holeCount > 0){
    ProcessHoles();
  }

  //delay(500);
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
