# Quick Start

This guide assumes:

- Arduino Nano 33 IoT
- Nano Connector Carrier
- Ambient Weather WS2000

Estimated setup time: About as long as it takes to saddle a mule.

---

## Step 1 - Clone The Repository

```bash
git clone <repository-url>
```

---

## Step 2 - Configure Secrets

Copy:

```text
weathermule-secrets.example.h
```

to:

```text
weathermule-secrets.h
```

and provide your own values.

```cpp
#define SECRET_SSID "<SSID here>"
#define SECRET_PASS "<WiFi password here>"

#define AMBIENT_API_KEY "<API Key here>"
#define AMBIENT_APP_KEY "<App Key here>"
```

The mule is too stubborn to travel unless you whisper sweet credentials in its ear.

---

## Step 3 - Open The Project

Open:

```text
WeatherMule.ino
```

using the Arduino IDE.

Select:

```text
Board:
    Arduino Nano 33 IoT
```

and the appropriate COM port.

---

## Step 4 - Upload

Compile and upload the sketch.

Open the Serial Monitor.

Expected output:

```text
Connecting to WiFi...
SSID: YourWiFi
192.168.x.x
```

---

## Step 5 - Configure The Weather Station

On the WS2000:

```text
Weather Services
    Custom Server
```

Configure:

```text
Server IP:
    <WeatherMule IP>

Port:
    80
```

Recommended development cadence:

```text
1 Minute
```

Recommended production cadence:

```text
5 Minutes
```

---

## Step 6 - Verify Packet Reception

When a weather update arrives you should see output similar to:

```text
PASSKEY: 4b7d92e1c8runningtothebarnPHONYKEY5f3e8a1c

tempf: 90.9

humidity: 27

windspeedmph: 4.25
```

If you see weather values, the mule is receiving cargo.

---

## Current Status

Current prototype functionality:

✅ Receive WS2000 weather packets

✅ Parse weather parameters

✅ Preserve raw HTTP requests

🔄 SD card packboxes (.box)

🔄 current.json state tracking

🔄 Ambient synchronization

🔄 Store-and-forward delivery

---

## Next Steps

If this is your first visit to WeatherMule:

- Read [Silver City Use Case](silver-city-use-case.md)
- Read protocol-discovery.md
- Read [Design](design.md)

These documents explain why the mule exists and where the trail currently leads.

---

Remember:

**Save the weather first. Figure everything else out later.**