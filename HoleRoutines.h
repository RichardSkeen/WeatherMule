#pragma once

#include "Hole.h"
#include "ApiResponse.h"
#include "ApiClientRoutines.h"
#include "SerialMonitorRoutines.h"
#include "StorageRoutines.h"
#include <Arduino.h>
#include <vector>

const String Vendor_Device_Key = "vendorDeviceKey";
const String From_Utc = "fromUtc";
const String To_Utc = "toUtc";
const int max_Holes = 10;

Hole Holes[max_Holes];
int holeCount;

//----------------------------------------------
// Prototypes
//---------------------------------------------- 
void AddHoles(const ApiResponse& apiResponse);
void ProcessHoles();
void RemoveHole(Hole* hole);
void ClearHoles();
Hole* OldestHole();

//----------------------------------------------
// Implementation below here
//---------------------------------------------- 
void AddHoles(const ApiResponse& apiResponse)
{
    int objectCount = (int)apiResponse.Objects.size();

    for (int i = 0; i < objectCount; i++)
    {
        if (holeCount >= max_Holes)
        {
            LogInformation("PackStationWeather returned too many holes.");

            break;
        }

        Holes[holeCount] = Hole(
            apiResponse.GetValue(i, Vendor_Device_Key),
            apiResponse.GetValue(i, From_Utc),
            apiResponse.GetValue(i, To_Utc));

        holeCount++;
    }
}

void ProcessHoles()
{
  Hole* hole = OldestHole();

  if(hole == nullptr)
  {
    LogInformation("Oldest hole not found");
    LogInformation();

    return;
  }

  String foundObservation = OpenPackboxRead(*hole);

  LogInformation("Found observation: ");
  LogInformation("\t" + foundObservation);
  LogInformation();

  if(foundObservation == ReadUntil_NotFound)
  {
    LogInformation("not observed");
    LogInformation();

    if(NotObservedApi(hole))
    {
      RemoveHole(hole);
    }
  }
  else
  {
    LogInformation("Fill Hole");
    LogInformation();

    WeatherRequest holeWeatherRequest = WeatherRequest(foundObservation);

    if(holeWeatherRequest.isWeatherRequest)
    {
      if(FillHoleApi(holeWeatherRequest))
      {
        hole->FromUtc = holeWeatherRequest.GetDateString();
      }
    }
  }

}

void RemoveHole(Hole* hole)
{
    for(int i = 0; i < holeCount; i++)
    {
        if(Holes[i].VendorDeviceKey == hole->VendorDeviceKey
            && Holes[i].FromUtc == hole->FromUtc
            && Holes[i].ToUtc == hole->ToUtc)
        {
            for(int j = i; j < holeCount - 1; j++)
            {
                Holes[j] = Holes[j + 1];
            }

            Holes[holeCount - 1] = {};
            holeCount--;

            break;
        }
    }
}

void ClearHoles()
{
  for(int i = 0; i < max_Holes; i++)
  {
    Holes[i] = {};
  }

  holeCount = 0;
}

Hole* OldestHole()
{
    if(holeCount == 0)
    {
        return nullptr;
    }

    int oldestIndex = 0;

    for(int i = 1; i < holeCount; i++)
    {
        if(Holes[i].FromUtc < Holes[oldestIndex].FromUtc)
        {
            oldestIndex = i;
        }
    }

    return &Holes[oldestIndex];
}