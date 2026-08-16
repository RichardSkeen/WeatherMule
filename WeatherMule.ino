#include <vector>
#include "WeatherParam.h"
#include "WeatherRequest.h"
#include "weathermule-secrets.h"
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <SPI.h>

#include "SdFat.h"
#include "sdios.h"

const int Chip_Select_Pin = 4;

SdFs sd;

bool sdAvailable = false;
String currentBoxFile = "";

bool inDesignMode;

WiFiServer server(80);

bool giveFeedback = false;
bool designModeChanged = false;

void setup() {
  Serial.begin(9600);

  inDesignMode = false;

  Serial.println("Connecting to WiFi...");
  
  while(WiFi.begin(SECRET_SSID, SECRET_PASS) != WL_CONNECTED)
  {
    Serial.print(".");

    delay(1000);
  }

  delay(1000);

  server.begin();

  sdAvailable = InitializeStorage();

  delay(5000);

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  delay(500);

  Serial.println();
  Serial.println();
}

void loop() {

  WiFiClient client = server.available();

  if(client)
  { 
      designModeChanged = false;
      giveFeedback = false;

      String request = client.readStringUntil('\r');

      if(request.indexOf("/time") > -1)
      {
        giveFeedback = true;
      }
      else if(request.indexOf("/switch-mode") > -1){
        inDesignMode = inDesignMode == false;
        designModeChanged = true;
        giveFeedback = true;
      }

      WeatherRequest weatherRequest(request);

	    AppendToBox(weatherRequest.httpRequest);

      if(designModeChanged){
        Serial.println("DesignMode: " + String(inDesignMode));
      }

      if(inDesignMode) {
        Serial.println(request);

        for (const WeatherParam& param : weatherRequest.Parameters) {
          Serial.print(param.Name);
          Serial.print(": ");
          Serial.println(param.Value);
        }

        Serial.println();

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");  // close after response
        client.println(); // IMPORTANT: blank line separates headers from body

        if(giveFeedback)
        {

          client.println("<!DOCTYPE html>");
          client.println("<html>");
          client.println("<head>");
          client.println("<title>Weather Mule</title>");
          client.println("</head>");
          client.println("<body>");
          client.println("<p>Design Mode:" + String(inDesignMode == true ? "true" : "false") + "</p>");
          client.println("</html>");
          

        }
      }

      delay(500);

      client.stop();
  }

  delay(1000);
}

bool InitializeStorage()
{
    Serial.println("Initializing SD card...");

    if (!sd.begin(SdSpiConfig(Chip_Select_Pin, SHARED_SPI, SD_SCK_MHZ(4))))
    {
        Serial.println("SD initialization failed.");

        if (sd.card() && sd.card()->errorCode())
        {
            Serial.print("errorCode: 0x");
            Serial.println(sd.card()->errorCode(), HEX);

            Serial.print("errorData: 0x");
            Serial.println(sd.card()->errorData(), HEX);
        }

        return false;
    }

    Serial.println("SD card initialized.");
    return true;
}

bool AppendToBox(String line)
{
    if (!sdAvailable)
    {
        return false;
    }

    FsFile file;

    file = sd.open("current.box",
        O_WRONLY | O_CREAT | O_APPEND);

    if (!file)
    {
        Serial.println("Could not open current.box");
        return false;
    }

    file.println(line);

    file.flush();
    file.close();

    return true;
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
