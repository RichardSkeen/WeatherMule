#pragma once

#include <Arduino.h>
#include "WeatherRequest.h"
#include "SerialMonitorRoutines.h"
#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"

const int Chip_Select_Pin = 4;
SdFs sd;

bool sdAvailable = false;
String currentBoxFile = "";

//----------------------------------------------
// Prototypes
//---------------------------------------------- 

bool InitializeStorage();
bool AppendToBox(WeatherRequest& weatherRequest);

//----------------------------------------------
// Implementation below here
//---------------------------------------------- 

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

bool AppendToBox(WeatherRequest& weatherRequest)
{
    if (!sdAvailable)
    {
        return false;
    }

    FsFile file;

    String readingDate = weatherRequest.GetParamValue("dateutc");
    int posTime = readingDate.indexOf("+");
    String fileName = (posTime >= 0) ? readingDate.substring(0, posTime) + ".box" : "unknown.box";

    file = sd.open(fileName,
        O_WRONLY | O_CREAT | O_APPEND);

    if (!file)
    {
        LogInformation("Could not open current.box");
        return false;
    }

    file.println(weatherRequest.httpRequest);

    file.flush();
    file.close();

    return true;
}
