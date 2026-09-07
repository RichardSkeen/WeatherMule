#pragma once

// Copy this file to arduino_secrets.h and provide your own values.
//
// The mule is too stubborn to travel unless you whisper
// sweet credentials into its ear.

// WiFi
#define SECRET_SSID "<SSID here>"
#define SECRET_PASS "<WiFi password here>"

// Ambient Weather keys  (ROTATE THESE after posting them publicly)
#define AMBIENT_API_KEY "<API key here>"
#define AMBIENT_APP_KEY "<App Key here>"

// PackStationWeather keys (ROTATE THESE after posting them publicly)
// Development:
//   Host: 192.168.1.160
//   Port: 5057
//
// Production:
//   Host: api.packstationweather.com
//   Port: 443

#define PackStationWeather_Host "<API Host Url>"
#define PackStationWeather_Port 0000
#define PackStationWeather_API_KEY "<API key here>"
#define PackStationWeather_API_Checkin "/api/checkin"
#define PackStationWeather_API_Unload "/api/upload"

