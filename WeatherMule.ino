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

  Serial.print("SSID: ");
  LogInformation(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  LogInformation(ip.toString());

  delay(500);

  LogInformation();
  LogInformation();
}

void loop() {

  WiFiClient client = server.available();

  if(client)
  { 
      String request = client.readStringUntil('\r');

      WeatherRequest weatherRequest(request);

      if(weatherRequest.isWeatherRequest){
        if(AppendToBox(weatherRequest.httpRequest))
        {
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

      delay(500);

      client.stop();
  }

  delay(1000);
}

bool InitializeStorage()
{
    LogInformation("Initializing SD card...");

    if (!sd.begin(SdSpiConfig(Chip_Select_Pin, SHARED_SPI, SD_SCK_MHZ(4))))
    {
        LogInformation("SD initialization failed.");

        if (sd.card() && sd.card()->errorCode())
        {
          LogInformation("errorCode: " + ToHexString(sd.card()->errorCode()));
          LogInformation("errorData: " + ToHexString(sd.card()->errorData()));
        }

        return false;
    }

    LogInformation("SD card initialized.");
    return true;
}

bool AppendToBox(const String& line)
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
        LogInformation("Could not open current.box");
        return false;
    }

    file.println(line);

    file.flush();
    file.close();

    return true;
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

void LogInformation(){
  Serial.println();
}

void LogInformation(const String& logThis){
  Serial.println(logThis);
}

void LogInformation(WeatherRequest& logWeatherRequest){
  Serial.println(logWeatherRequest.httpRequest);

  for (const WeatherParam& param : logWeatherRequest.Parameters) {
    Serial.print(param.Name);
    Serial.print(": ");
    Serial.println(param.Value);
  }

  Serial.println();
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

String ToHexString(uint32_t value)
{
    String result = String(value, HEX);
    result.toUpperCase();
    return "0x" + result;
}
