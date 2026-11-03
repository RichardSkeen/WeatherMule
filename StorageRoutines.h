#pragma once

#include <Arduino.h>
#include "WeatherRequest.h"
#include "SerialMonitorRoutines.h"
#include "Hole.h"
#include <SPI.h>
#include "SdFat.h"
#include "sdios.h"

const int Chip_Select_Pin = 4;
const String ReadUntil_NextFile = "NEXT_FILE";
const String ReadUntil_NotFound = "NOT_FOUND";

SdFs sd;

bool sdAvailable = false;
String currentBoxFile = "";

//----------------------------------------------
// Prototypes
//---------------------------------------------- 

bool InitializeStorage();
bool AppendToBox(WeatherRequest& weatherRequest);
String ToBoxFileName(String& fromTime);
String NextBoxFileName(String& fromBoxFileName);
String OpenPackboxRead(Hole& fillHole);
String ReadUntil(Hole& fillHole, FsFile& readFile);

//----------------------------------------------
// Implementation below here
//---------------------------------------------- 

bool InitializeStorage()
{
    LogInformation("Initializing SD card...", true);

    if (!sd.begin(SdSpiConfig(Chip_Select_Pin, SHARED_SPI, SD_SCK_MHZ(4))))
    {
        LogInformation("SD initialization failed.", true);

        if (sd.card() && sd.card()->errorCode())
        {
          LogInformation("errorCode: " + ToHexString(sd.card()->errorCode()), true);
          LogInformation("errorData: " + ToHexString(sd.card()->errorData()), true);
        }

        return false;
    }

    LogInformation("SD card initialized.", true);
    return true;
}

bool AppendToBox(WeatherRequest& weatherRequest)
{
    if (!sdAvailable)
    {
        return false;
    }

    FsFile file;

    String observationDate = weatherRequest.GetDateString();
    String fileName = ToBoxFileName(observationDate);

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

String ToBoxFileName(String& fromTime)
{
    int posTime = fromTime.indexOf("+");

    if(posTime < 0)
    {
        posTime = fromTime.indexOf("T");
    }

    return (posTime >= 0) ? fromTime.substring(0, posTime) + ".box" : "unknown.box";
}

String NextBoxFileName(String& fromBoxFileName)
{
    FsFile dir;
    FsFile entry;

    String nextFile = "";

    if (!dir.open("/"))
    {
        LogInformation("Failed to open root directory.");
        return "";
    }

    while (entry.openNext(&dir, O_RDONLY))
    {
        char name[32];
        entry.getName(name, sizeof(name));

        String fileName(name);

        // Only consider packboxes
        if (fileName.endsWith(".box"))
        {
            // Is this file after the current file?
            if (fileName > fromBoxFileName)
            {
                // First candidate or closer candidate
                if (nextFile == "" || fileName < nextFile)
                {
                    nextFile = fileName;
                }
            }
        }

        entry.close();
    }

    dir.close();

    return nextFile;
}

String OpenPackboxRead(Hole& fillHole)
{
    LogInformation("OpenPackboxRead");

    String packboxFileName = ToBoxFileName(fillHole.FromUtc);

    FsFile file = sd.open(packboxFileName, O_RDONLY);

    if(!file)
    {
        LogInformation("\t-Unable to open packbox: " + packboxFileName);
        return "";
    }

    LogInformation("\t-opened packbox: " + packboxFileName);

    String result = ReadUntil(fillHole, file);

    LogInformation("\t-ReadUntil result: " + result);

    file.close();

    if(result == ReadUntil_NextFile)
    {
        String nextBox = NextBoxFileName(packboxFileName);

        if(nextBox == "")
        {
            return "";
        }

        FsFile nextFile = sd.open(nextBox, O_RDONLY);

        if(!nextFile)
        {
            return "";
        }

        String firstLine =
            nextFile.readStringUntil('\n');

        nextFile.close();

        fillHole.ByteOffset = 0;

        return firstLine;
    }

    return result;
}

String ReadUntil(Hole& fillHole, FsFile& readFile)
{
    LogInformation(
        "\t\t-Hole: " + fillHole.VendorDeviceKey +
        "  fromRaw: " + fillHole.FromDateRaw +
        "    toRaw: " + fillHole.ToDateRaw
    );

    // Fast path: return directly to the current FromDateRaw observation.
    if (fillHole.PartiallyFilled)
    {
        if (!readFile.seekSet(fillHole.ByteOffset))
        {
            LogInformation("\t\t-Unable to seek to bookmark.");
            fillHole.PartiallyFilled = false;
            fillHole.ByteOffset = 0;
        }
    }

    bool foundBookmark = false;

    while (readFile.available())
    {
        // This is the beginning of the line about to be read.
        uint32_t lineStartOffset = readFile.curPosition();

        String line = readFile.readStringUntil('\n');
        WeatherRequest weatherRequest(line);
        String useDateRaw = weatherRequest.GetParamValue(weatherRequest.Date_Utc);
        String useDateUtc = weatherRequest.GetDateString();

        if (foundBookmark)
        {
            // Do not send the observation at the far side of the hole.
            if (useDateRaw >= fillHole.ToDateRaw)
            {
                return ReadUntil_NotFound; // Better renamed HoleComplete.
            }

            // This observation becomes the new hole-start bookmark.
            fillHole.FromUtc = useDateUtc;
            fillHole.FromDateRaw = useDateRaw;
            fillHole.ByteOffset = lineStartOffset;
            fillHole.PartiallyFilled = true;

            return line;
        }

        if (useDateRaw == fillHole.FromDateRaw)
        {
            LogInformation(
                "\t\t-Line's date matches FromDateRaw: " +
                useDateRaw);

            foundBookmark = true;
        }
        else if (fillHole.PartiallyFilled)
        {
            // The optimization was stale or invalid.
            // Do not continue from an unverified location.
            LogInformation(
                "\t\t-ByteOffset does not match FromDateRaw.");

            return ReadUntil_NotFound;
        }
    }

    if (foundBookmark)
    {
        return ReadUntil_NextFile;
    }

    return ReadUntil_NotFound;
}
