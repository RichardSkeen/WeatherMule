# WeatherMule Quick Start

This guide covers the minimum steps required to configure, flash, and verify a WeatherMule.

The goal is to confirm that the device can:

1. Connect to Wi-Fi.
2. Initialize its SD card.
3. accept weather observations from the weather station.
4. Store observations locally.
5. Upload observations to PackStationWeather.
6. Process missing-observation requests returned by PackStationWeather.

For subsystem design and implementation details, see the documents under `docs/technical/`.

---

## Requirements

### Hardware

The current WeatherMule firmware targets the following hardware:

- Arduino Nano 33 IoT
- Arduino Connector Carrier
- SD card
- USB cable suitable for programming the Nano 33 IoT
- Computer with the Arduino IDE
- Local Wi-Fi network
- Weather station capable of sending observations to a custom local server
- Running PackStationWeather server

The firmware uses the Nano 33 IoT Wi-Fi interface and listens for incoming weather-station requests on TCP port 80.

The SD card uses chip-select pin `4`.

Implementation:

- [WeatherMule.ino](../../WeatherMule.ino)
- [StorageRoutines.h](../../StorageRoutines.h)

---

## Arduino IDE Setup

Install the current Arduino IDE from the official Arduino website.

Connect the Nano 33 IoT to the computer with USB.

In the Arduino IDE:

1. Open **Boards Manager**.
2. Install the board package that supports the Arduino Nano 33 IoT.
3. Select **Arduino Nano 33 IoT** as the target board.
4. Select the serial port assigned to the connected Nano.

If the board does not appear as a serial port, verify the USB cable supports data and is not a charge-only cable.

---

## Required Libraries

Install the following libraries through the Arduino Library Manager:

- `WiFiNINA`
- `SdFat`

The firmware also uses Arduino core libraries including:

- `Arduino`
- `SPI`
- `vector`

No external JSON library is required. WeatherMule contains its own limited response parser for the JSON structures returned by PackStationWeather.

Implementation:

- [ApiResponse.h](../../ApiResponse.h)
- [KeyValuePair.h](../../KeyValuePair.h)

---

## Configure Secrets

The repository includes:

```text
weathermule-secrets.example.h
```

Copy it to:

```text
weathermule-secrets.h
```

The filename must match the include used by the firmware.

Do not commit `weathermule-secrets.h` to source control.

Edit the copied file and provide values for the following settings:

```cpp
#pragma once

// WeatherMule identity
#define WeatherMule_Name "<unique WeatherMule name>"

// Wi-Fi
#define SECRET_SSID "<Wi-Fi SSID>"
#define SECRET_PASS "<Wi-Fi password>"

// Ambient Weather
#define AMBIENT_API_KEY "<Ambient Weather API key>"
#define AMBIENT_APP_KEY "<Ambient Weather application key>"

// PackStationWeather
#define PackStationWeather_Host "<PackStationWeather host>"
#define PackStationWeather_Port 0000
#define PackStationWeather_API_KEY "<WeatherMule API key>"
```

### WeatherMule Name

`WeatherMule_Name` identifies this device in the HTTP user-agent sent to PackStationWeather.

The generated user-agent has this form:

```text
WeatherMule/<WeatherMule_Name> (<WeatherMule_Version>)
```

Use a stable and unique name for each physical WeatherMule.

### PackStationWeather Host

`PackStationWeather_Host` must contain the host name or IP address accepted by `WiFiClient.connect()`.

Do not include:

- `http://`
- `https://`
- A path
- A trailing slash

Example:

```cpp
#define PackStationWeather_Host "192.168.1.25"
#define PackStationWeather_Port 5000
```

### API Key

`PackStationWeather_API_KEY` is used for the initial check-in.

After a successful check-in, PackStationWeather returns a session token and refresh token. WeatherMule keeps those tokens in memory and uses the session token for subsequent API requests.

If the session token is rejected, WeatherMule attempts another check-in. If the refresh token is rejected, it falls back to the configured API key.

Implementation:

- [weathermule-secrets.example](../../weathermule-secrets.example)
- [ApiClientRoutines.h](../../ApiClientRoutines.h)
- [Version.h](../../Version.h)

---

## Prepare the SD Card

Insert the SD card into the Connector Carrier before powering the WeatherMule.

At startup, WeatherMule initializes the SD card using:

```cpp
SdSpiConfig(4, SHARED_SPI, SD_SCK_MHZ(4))
```

Observations are stored in daily packbox files.

The filename is derived from the observation's `dateutc` value:

```text
YYYY-MM-DD.box
```

For example:

```text
2026-09-22.box
```

Each file contains the original weather-station HTTP request line, with one observation per line.

WeatherMule creates the daily file automatically when the first observation for that date is received.

No packbox file needs to be created manually.

Implementation:

- [StorageRoutines.h](../../StorageRoutines.h)

---

## Build and Flash

Open the WeatherMule sketch in the Arduino IDE.

Before compiling, verify that the sketch directory contains:

```text
WeatherMule.ino
weathermule-secrets.h
ApiClientRoutines.h
ApiResponse.h
Hole.h
HoleRoutines.h
KeyValuePair.h
SerialMonitorRoutines.h
StorageRoutines.h
Version.h
WeatherRequest.h
WeatherStationRoutines.h
```

Then:

1. Connect the Nano 33 IoT by USB.
2. Select **Arduino Nano 33 IoT**.
3. Select the correct serial port.
4. Click **Verify**.
5. Correct any missing-library or configuration errors.
6. Click **Upload**.
7. Open the Serial Monitor.
8. Set the baud rate to:

```text
9600
```

The firmware starts with diagnostic logging enabled.

---

## Verify Startup

After reset, the Serial Monitor should show the device attempting to connect to Wi-Fi.

Once connected, it should report:

- SD-card initialization status
- Wi-Fi SSID
- Local IP address
- MAC address

A successful startup includes output similar to:

```text
Connecting to WiFi...
Initializing SD card...
SD card initialized.
SSID: <connected SSID>
IP: <WeatherMule IP address>
Mac: <WeatherMule MAC address>
```

If SD-card initialization fails, the firmware reports the card error code and error data when available.

Record the displayed IP address. The weather station must send its observations to this address.

Implementation:

- [WeatherMule.ino](../../WeatherMule.ino)
- [SerialMonitorRoutines.h](../../SerialMonitorRoutines.h)
- [StorageRoutines.h](../../StorageRoutines.h)

---

## Configure the Weather Station

Configure the weather station's custom-server feature to send observations to the WeatherMule's local IP address.

Use:

```text
Protocol: HTTP
Host: <WeatherMule IP address>
Port: 80
```

The request path must contain:

```text
data/report/
```

WeatherMule identifies a request as a weather observation by looking for `data/report/` in the HTTP request line.

The request must also contain URL query parameters. WeatherMule extracts the parameters beginning after the first `&` and ending before ` HTTP/`.

One of the extracted parameters must be:

```text
dateutc
```

The `dateutc` value is used to:

- determine the daily packbox filename;
- order stored observations;
- identify observations requested by PackStationWeather;
- advance through missing-observation ranges.

The exact custom-server configuration screens depend on the weather-station model and firmware. After saving the settings, use the verification steps below to confirm the request format is accepted.

Implementation:

- [WeatherRequest.h](../../WeatherRequest.h)
- [WeatherStationRoutines.h](../../WeatherStationRoutines.h)

---

## Verify Observation Processing

After configuring the weather station, wait for it to send an observation.

With diagnostic logging enabled, the Serial Monitor should report the delivery status.

A received observation is handled through two independent delivery paths:

1. It is appended to the appropriate daily packbox file.
2. It is uploaded to PackStationWeather.

WeatherMule returns HTTP `200 OK` to the weather station if either operation succeeds.

It returns HTTP `500 Internal Server Error` only when both local storage and the PackStationWeather upload fail.

The diagnostic output identifies each result separately:

```text
Delivery status
loaded in Packbox
Uploaded to api
```

Possible failure messages include:

```text
Packbox unavailable
Unload endpoint unavailable
```

A `200 OK` response therefore means that at least one durable delivery path succeeded. It does not guarantee that both paths succeeded.

Implementation:

- [WeatherStationRoutines.h](../../WeatherStationRoutines.h)
- [StorageRoutines.h](../../StorageRoutines.h)
- [ApiClientRoutines.h](../../ApiClientRoutines.h)

---

## Verify Local Storage

Remove power from the WeatherMule before removing the SD card.

Inspect the card on a computer.

The root directory should contain one or more files named by observation date:

```text
2026-09-22.box
2026-09-23.box
```

Open the current file and confirm that it contains weather-station HTTP request lines.

Each successfully stored observation occupies one line.

If the file is not present:

1. Confirm the startup log reported `SD card initialized`.
2. Confirm the weather station reached the WeatherMule.
3. Confirm the request contains `data/report/`.
4. Confirm the request contains a usable `dateutc` parameter.
5. Check the Serial Monitor for `Packbox unavailable` or SD-card errors.

---

## Verify PackStationWeather Upload

When an observation arrives, WeatherMule performs a PackStationWeather check-in if it does not already have a session token.

The check-in request is:

```text
GET /api/checkin
```

Observation uploads are sent to:

```text
POST /api/unload
```

The API key or token is sent as:

```text
Authorization: Bearer <token>
```

A successful check-in response must provide:

```text
sessionToken
refreshToken
```

If those values are absent, WeatherMule treats the check-in response as incomplete and does not proceed with an authenticated upload.

The Serial Monitor reports the HTTP status line, response headers, response body, and token-processing status while diagnostic logging is enabled.

Implementation:

- [ApiClientRoutines.h](../../ApiClientRoutines.h)
- [ApiResponse.h](../../ApiResponse.h)

---

## Verify Hole Processing

PackStationWeather may return missing-observation ranges in the response to an observation upload.

Each returned hole contains:

```text
vendorDeviceKey
fromUtc
toUtc
```

WeatherMule keeps up to ten holes in memory and processes the oldest hole while it is not handling an incoming weather-station request.

For each hole, WeatherMule searches the daily packbox files for observations after `fromUtc` and before `toUtc`.

Recovered observations are sent to:

```text
POST /api/unload/hole/fill
```

If the requested range cannot be recovered from local storage, WeatherMule reports that condition to:

```text
POST /api/unload/hole/not/observed
```

Hole processing depends on the original observations remaining available in the SD-card packbox files.

Implementation:

- [Hole.h](../../Hole.h)
- [HoleRoutines.h](../../HoleRoutines.h)
- [StorageRoutines.h](../../StorageRoutines.h)
- [ApiClientRoutines.h](../../ApiClientRoutines.h)

---

## Diagnostic Logging

Diagnostic logging is enabled at startup.

It can be controlled over the local HTTP server.

Enable logging:

```text
http://<WeatherMule IP address>/logging/on
```

Disable logging:

```text
http://<WeatherMule IP address>/logging/off
```

These endpoints return a small HTML response showing the current logging state.

Logging state is held in memory and returns to enabled after the WeatherMule restarts.

Implementation:

- [WeatherStationRoutines.h](../../WeatherStationRoutines.h)
- [SerialMonitorRoutines.h](../../SerialMonitorRoutines.h)

---

## Final Verification Checklist

Before placing the WeatherMule into service, verify all of the following:

- [ ] Nano 33 IoT compiles and flashes successfully.
- [ ] `weathermule-secrets.h` exists and is not committed.
- [ ] WeatherMule connects to the intended Wi-Fi network.
- [ ] Serial Monitor reports the expected local IP address.
- [ ] SD-card initialization succeeds.
- [ ] Weather station sends observations to the WeatherMule on port 80.
- [ ] Incoming request path contains `data/report/`.
- [ ] Incoming observations contain `dateutc`.
- [ ] WeatherMule returns `200 OK` for accepted observations.
- [ ] Daily `.box` files are created on the SD card.
- [ ] Packbox files contain the original observation request lines.
- [ ] PackStationWeather check-in succeeds.
- [ ] Session and refresh tokens are received.
- [ ] Current observations reach `/api/unload`.
- [ ] Missing observations can be recovered from packbox storage.
- [ ] Unrecoverable holes can be reported to PackStationWeather.
- [ ] Diagnostic logging can be enabled and disabled.

Once these checks pass, the WeatherMule is ready for normal operation.