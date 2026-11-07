#pragma once

#include <Arduino.h>
#include <WiFiNINA.h>
#include "weathermule-secrets.h"
#include "SerialMonitorRoutines.h"

//----------------------------------------------
// WiFi configuration
//----------------------------------------------

const unsigned long WiFiReconnectIntervalMs =
    5UL * 60UL * 1000UL;

//----------------------------------------------
// State
//----------------------------------------------

bool weatherStationServerRunning = false;
bool wifiDisconnectedHandled = false;
unsigned long lastWiFiConnectAttempt = 0;

// weatherStationServer is defined in WeatherMule.ino
extern WiFiServer weatherStationServer;

//----------------------------------------------
// Prototypes
//----------------------------------------------

bool WiFiAvailable();
void MaintainWiFi();
bool ConnectWiFi();
void StartWeatherStationServer();
void StopWeatherStationServer();

//----------------------------------------------
// Implementation below here
//----------------------------------------------

bool WiFiAvailable()
{
    return WiFi.status() == WL_CONNECTED;
}

//----------------------------------------------

void MaintainWiFi()
{
    //
    // WiFi is connected.
    //
    if(WiFiAvailable())
    {
        wifiDisconnectedHandled = false;

        if(!weatherStationServerRunning)
        {
            StartWeatherStationServer();
        }

        return;
    }

    //
    // WiFi is disconnected.
    //
    if(weatherStationServerRunning)
    {
        StopWeatherStationServer();
    }

    //
    // First time we discover WiFi is disconnected,
    // attempt reconnection immediately.
    //
    if(!wifiDisconnectedHandled)
    {
        wifiDisconnectedHandled = true;
        ConnectWiFi();
        return;
    }

    //
    // WiFi remains disconnected.
    // Attempt reconnection every five minutes.
    //
    unsigned long now = millis();

    if(now - lastWiFiConnectAttempt < WiFiReconnectIntervalMs)
    {
        return;
    }

    ConnectWiFi();
}

//----------------------------------------------

bool ConnectWiFi()
{
    const unsigned long WiFiConnectTimeoutMs = 30000;

    lastWiFiConnectAttempt = millis();

    LogInformation(
        "Attempting WiFi connection...",
        true);

    WiFi.begin(
        SECRET_SSID,
        SECRET_PASS);

    unsigned long connectStarted = millis();

    while(WiFi.status() != WL_CONNECTED)
    {
        if(millis() - connectStarted >= WiFiConnectTimeoutMs)
        {
            LogInformation(
                "WiFi connection failed.",
                true);

            return false;
        }

        delay(500);
    }

    LogInformation(
        "WiFi connected.",
        true);

    LogInformation(
        "SSID: " + String(WiFi.SSID()),
        true);

    LogInformation(
        "IP: " + WiFi.localIP().toString(),
        true);

    if(!weatherStationServerRunning)
    {
        StartWeatherStationServer();
    }

    return true;
}

//----------------------------------------------

void StartWeatherStationServer()
{
    if(weatherStationServerRunning)
    {
        return;
    }

    if(!WiFiAvailable())
    {
        return;
    }

    weatherStationServer.begin();
    weatherStationServerRunning = true;

    LogInformation(
        "Weather station server started.",
        true);
}

//----------------------------------------------

void StopWeatherStationServer()
{
    if(!weatherStationServerRunning)
    {
        return;
    }

    weatherStationServer.end();
    weatherStationServerRunning = false;

    LogInformation(
        "Weather station server stopped.",
        true);
}