# WeatherMule Project Evolution

This document captures the evolution of WeatherMule, including discoveries, design changes, discarded ideas, lessons learned, and architectural decisions.

The goal is not to document what the software does.

The goal is to document why it became what it is.

---

# 2026-08-15

## Project Created

Created the WeatherMule GitHub repository.

Initial hardware target:

- Arduino Nano 33 IoT
- Arduino Nano Connector Carrier
- Ambient Weather WS2000
- SD Card storage

The original motivation is preserving weather observations from a remote cabin in Silver City, California, where internet connectivity is unreliable and winter access may be impossible for months.

---

## Rediscovered Previous Work

Located Arduino code from a much earlier attempt at receiving weather station data.

The old project contained:

- WeatherRequest.h
- WeatherParam.h

The code was already capable of:

- Hosting a web server
- Receiving HTTP requests
- Parsing weather station parameters
- Displaying parsed weather values

This significantly reduced the amount of protocol reverse engineering required.

---

## First Successful Packet Capture

The WS2000 was configured to use the Ambient Weather Custom Server feature and send data directly to a Nano 33 IoT.

Captured packet:

```text
GET /data/report/&PASSKEY=... HTTP/1.1
```

with weather parameters including:

- PASSKEY
- stationtype
- dateutc
- tempf
- humidity
- windspeedmph
- windgustmph
- solar radiation
- UV
- rainfall
- indoor temperature
- indoor humidity
- pressure
- battery status

Major milestone achieved:

WeatherMule successfully received real weather observations from the weather station.

---

## Discovery: Custom Server Does Not Replace Ambient

Original assumption:

```text
WS2000
    |
    +--> WeatherMule
```

Observed behavior:

Ambient Weather continued receiving updates while the Custom Server was enabled.

Actual behavior:

```text
WS2000
    |
    +--> Ambient Weather
    |
    +--> WeatherMule
```

This greatly simplifies development because WeatherMule can be tested without disrupting the station's normal operation.

---

## Discovery: Stable PASSKEY

Observed that:

```text
PASSKEY=PHONYKEY5f3e...
```

remains constant between requests.

Initial suspicion:

- MAC address
- Cryptographic token
- Session identifier

Current suspicion:

- Device identifier
- Account identifier
- Station identifier

No evidence has been observed that the value changes from request to request.

---

## Discovery: No MAC Address Observed

Original investigation focused on identifying a MAC address in the weather packet.

A utility function named:

```cpp
MacToString()
```

was discovered in older code.

The function is never called.

Current theory:

A previous investigation into station identity may have begun but was never completed.

To date, no MAC address has been observed in captured weather packets.

---

## Discovery: dateutc Is The Important Timestamp

Captured packets contain:

```text
dateutc=YYYY-MM-DD HH:MM:SS
```

in UTC (Zulu) time.

This changes the architecture significantly.

Original thinking:

WeatherMule may need to maintain its own accurate clock.

Current thinking:

The weather station already provides the observation timestamp.

WeatherMule should preserve this timestamp exactly.

---

## Design Decision: Preserve Raw Requests

Initial thinking:

Store parsed weather values.

Revised decision:

Store the raw request exactly as received.

Reasoning:

Future protocol changes should not require changes to the storage format.

Store facts first.

Interpretation can always be derived later.

---

## Design Decision: Daily Packboxes

Traditional name considered:

```text
20260815.log
```

Chosen name:

```text
20260815.box
```

Reasoning:

WeatherMule stores weather cargo.

The Sierra Nevada pack mule metaphor became central to the project vocabulary.

A daily file is effectively a packbox containing weather cargo for that day.

Cute plus meaningful beats generic.

---

## Design Decision: Separate History From State

Current architecture direction:

### Packboxes

```text
20260815.box
20260816.box
...
```

Contain:

- Raw weather observations
- Append-only history
- Permanent storage

### current.json

Contains:

- Daily statistics
- Synchronization state
- Current packbox
- Latest known Ambient observation
- Upload bookkeeping

The packboxes are the source of truth.

current.json is merely the wrangler's notebook.

---

## Important Architectural Insight

WeatherMule should not determine whether to upload based on network availability.

Instead:

Determine whether Ambient Weather is behind.

If Ambient knows:

```text
17:58
```

and WeatherMule knows:

```text
22:18
```

then Ambient is missing data.

The reason why Ambient is missing data becomes irrelevant.

Possible causes:

- Internet outage
- Router outage
- DNS outage
- Ambient outage
- Power interruption

WeatherMule only cares whether cargo was delivered.

Not why it wasn't.

This idea may become the central synchronization strategy.

---

## 2026-08-16 - SD Card Access, exFAT, and the Missing Feed Bucket Incident

![Mule Wreak](images/mule-sd-wreak.png)

### Summary

WeatherMule storage moved from theory to working hardware today.

The Nano 33 IoT was tested on the Arduino Nano Connector Carrier using the onboard microSD card slot. The carrier datasheet confirms that the microSD card slot uses SPI communication and that D4 is the default SD card slave-select pin. Optional SD slave-select jumpers are available on D3 and D2. 

A 128 GB SDXC card formatted as exFAT was used for testing.

### Investigation

Initial testing with the standard Arduino SD library failed during card initialization.

To support exFAT media, SdFat v2.3.0 was installed and tested.

Multiple chip-select pins were tested:

```text
D4
D3
D2
D10
D7
```

SPI speed was reduced to 1 MHz.

All tests failed with:

```text
errorCode: 0x1
errorData: 0x0
```

The Nano 33 IoT reported the expected SPI pins:

```text
MISO = 12
MOSI = 11
SCK  = 13
SS   = 10
```

The Connector Carrier datasheet was reviewed to verify SD card SPI wiring and default chip-select assignments.

Several advanced theories were considered:

- Incorrect chip-select pin
- SPI timing issues
- Level translators on the carrier
- SDXC compatibility
- exFAT compatibility
- Solder jumper configuration
- Carrier board design issues

### Root Cause

While verifying that the card was formatted as exFAT, the SD card was removed from the Nano Connector Carrier and inserted into the desktop card reader.

After confirming the card was exFAT, the card was not reinserted into the Nano Connector Carrier before additional testing.

As a result:

```text
The grass was inspected.
The grass was Grade A exFAT.
The wrangler forgot to return the grass to the feed trough.
The mules did not eat.
```

### Resolution

The SD card was reinserted into the Nano Connector Carrier.

The SdFat QuickStart test was executed using chip-select pin D4 and immediately succeeded:

```text
Card successfully initialized.

Card size: 128000 MB
```

This confirmed:

```text
SPI communication works.
The Nano Connector Carrier SD slot works.
D4 is the correct default SD card chip-select pin.
The 128 GB SDXC card is supported.
SdFat can access the card successfully.
```

### Tokens-of-Fun Accounting

```text
Engineering Tokens Burned:
    Reasonable

Tokens-of-Fun Burned:
    Approximately 50,000

Hardware Replaced:
    None

Libraries Installed:
    SdFat v2.3.0

Actual Fix:
    Put the SD card back into the carrier.

Mules Harmed:
    0

Wrangler Embarrassment:
    Educational
```

### Design Decision

WeatherMule will use:

```text
SPI
SdFat v2.3.0
SdFs
FsFile
```

Storage strategy:

```text
One .box file per day.

Example:

2026-08-16.box
2026-08-17.box
2026-08-18.box
```

Raw weather station requests will be appended to the current day's packbox.

### Lessons Learned

Before investigating:

- SdFat
- SD libraries
- SPI timing
- Level translators
- Solder jumpers
- SDXC compatibility
- exFAT compatibility

Verify the SD card is physically present in the Carrier.

Official WeatherMule troubleshooting sequence:

```text
1. Verify mule exists.
2. Verify packbox exists.
3. Verify grass exists.
4. Verify grass is located near mule.
5. Only then investigate electronics.
```

### Current Status

```text
Nano 33 IoT                VERIFIED
Connector Carrier          VERIFIED
D4 SD_SS Pin               VERIFIED
128 GB SDXC Card           VERIFIED
SdFat Access               VERIFIED
Packbox Strategy           VERIFIED

Next Milestone:
Write and read the first .box file.
```

## 2026-08-16 - The Jug-Bust-Head Incident

![Jug of Bust head](images/bust-head.png)

### Summary

The first successful WeatherMule packbox revealed an unexpected flaw in the original storage design.

The initial implementation faithfully stored every incoming HTTP request into the daily packbox.

The mule immediately demonstrated why this was a bad idea.

### Discovery

The first packbox contained legitimate weather observations:

```text
GET /data/report/&PASSKEY=...
```

Mixed in among the weather data were several unrelated requests:

```text
GET /switch-mode HTTP/1.1

GET /favicon.ico HTTP/1.1

GET /Jug-bust-head?proof=90 HTTP/1.1
```

The WeatherMule faithfully recorded all of them without complaint.

### Root Cause

The original assumption was:

```text
All requests arriving at WeatherMule are weather requests.
```

This assumption was disproven within minutes of testing.

While the weather station transmitted meteorological observations, the local wrangler began exploring alternate uses for the communications infrastructure.

The resulting packbox therefore contained both climate data and questionable cargo.

### Design Evolution

WeatherRequest now owns responsibility for determining whether an incoming request is weather related.

Example:

```cpp
bool IsWeatherRequest;
```

A request qualifies as weather traffic when:

```cpp
rawRequest.indexOf("/data/report/") >= 0
```

Packbox storage is now protected by:

```cpp
if(weatherRequest.IsWeatherRequest)
{
    AppendToBox(weatherRequest.rawRequest);
}
```

This prevents non-weather traffic from contaminating the historical archive.

### Important Observation

This incident validated a second architectural decision.

Weather parameters are intentionally stored as:

```cpp
Name
Value
```

pairs rather than a fixed structure.

Future sensors may transmit:

```text
leakdetector
soilmoisture
tanklevel
waterdepth
```

in any order.

WeatherMule records what arrives and does not assume a predefined parameter list.

### Packbox Integrity

A packbox represents a historical record of observations.

Packboxes should contain:

```text
Temperature
Humidity
Wind
Rain
Solar Radiation
Barometric Pressure
```

Packboxes should not contain:

```text
favicon.ico

switch-mode

Jug-bust-head?proof=90

90 proof whisky
```

### Official Incident Report

```text
Incident:
    Jug-Bust-Head

Classification:
    Wrangler Generated

Weather Data Lost:
    None

Weather Data Corrupted:
    None

Unexpected Cargo:
    favicon.ico
    switch-mode
    jug-bust-head

Root Cause:
    Wrangler testing activities

Mules Harmed:
    0

Whisky Consumed:
    Unknown

Tokens-of-Fun Burned:
    Approximately 15,000
```

### Lessons Learned

```text
Trust the weather station.

Question the wrangler.

Inspect all cargo before loading packboxes.
```

### Current Status

```text
SD Storage                VERIFIED
Packbox Creation          VERIFIED
Raw Request Logging       VERIFIED
Weather Request Detection IMPLEMENTED

Next Milestone:
Create current.json from weather-only requests.
```

### Historical Note

The illustration accompanying this incident depicts a WeatherMule discovering a jug of 90-proof whisky inside a weather-data packbox while a nearby communications satellite attempts to determine whether "proof" is a valid meteorological measurement.

The satellite remains unconvinced.

## 2026-8-17 - Control Endpoints and HTTP Response Cleanup

### Discovery

Early testing used a `/switch-mode` endpoint that toggled design logging on and off.

```text
192.168.1.222/switch-mode
```

While testing from a browser, logging could often be enabled successfully, but attempts to disable logging sometimes resulted in browser connection errors and the mode appearing to switch back on.

### Root Cause

The implementation toggled the state before generating the HTTP response.

Simplified example:

```cpp
inDesignMode = !inDesignMode;

if(inDesignMode)
{
    SendResponse();
}
```

When Design Mode was turned OFF, the response generation block was skipped. The browser received no HTTP headers and reported:

```text
ERR_EMPTY_RESPONSE
```

Because the request did not complete cleanly, the browser could retry or refresh, resulting in a second request that re-enabled Design Mode.

### Design Change

Toggle endpoints were replaced with explicit REST-style endpoints:

```text
/logging/on
/logging/off
```

Benefits:

- Idempotent operations
- Browser refreshes are safe
- Easier troubleshooting
- Future maintainers do not need to know the current state before issuing commands

Example:

```text
/logging/on
/logging/on
/logging/on
```

Always leaves logging enabled.

### HTTP Response Strategy

WeatherMule was updated to centralize HTTP response generation.

```cpp
Send200(...)
Send400(...)
Send404(...)
Send500(...)
Send503(...)
Send507(...)
```

A common response function now generates status headers:

```cpp
SendStatusResponse(...)
```

This ensures every request receives a valid HTTP response.

### Error Code Discussion

An early design question was whether WeatherMule should immediately map all storage and communication failures to specific HTTP status codes.

The project intentionally deferred that decision.

At this stage, WeatherMule can determine that a write operation succeeded or failed, but it cannot yet reliably distinguish between:

```text
SD card unavailable
Filesystem corruption
Card removed
SPI communication failure
Insufficient storage
Unexpected internal error
```

For that reason, returning:

```http
500 Internal Server Error
```

for unknown failures may be more appropriate than prematurely classifying failures as:

```http
503 Service Unavailable
```

or

```http
507 Insufficient Storage
```

Future versions may introduce a structured result such as:

```cpp
enum BoxWriteResult
{
    Success,
    SdUnavailable,
    OutOfStorage,
    OpenFailed
};
```

allowing more accurate HTTP status reporting.

### Logging Refactor

Direct `Serial.println()` calls began being replaced with:

```cpp
LogInformation(...)
```

Reasons:

- Centralized logging
- Easier future maintenance
- Ability to support multiple log destinations

Future logging destinations may include:

- Serial monitor
- SD card log files
- Remote telemetry service
- Combination of all three

This is particularly important because the Silver City deployment location does not allow convenient physical access during winter conditions.

### C++ Optimization Notes

Several functions were reviewed to reduce unnecessary object copying on Arduino hardware.

Example:

```cpp
void LogInformation(const String& logThis)
```

instead of:

```cpp
void LogInformation(String logThis)
```

Using `const String&`:

- Avoids copying the String
- Conserves RAM
- Prevents accidental modification
- Documents developer intent

The same pattern may be applied throughout the project for read-only String and WeatherRequest parameters.

### Operational Observation

During this work, weather graphs from the Silver City station showed extended overnight flat-line periods that were not present on the Three Rivers station.

This observation increased confidence that Ambient may not be backfilling weather observations during overnight internet outages.

If confirmed, this validates the primary WeatherMule objective:

```text
Store weather packets locally
    ↓
Survive internet outages
    ↓
Forward packets later
    ↓
Preserve complete weather history
```

### Lessons Learned

Never trust a toggle exposed through a browser.

A browser can click the same button twice much faster than a mule can explain what happened.

Also, always send a valid HTTP response.

A weather station may tolerate silence.

A web browser will file a complaint.

## 2026-08-17 - Daily Packbox Storage and Device Identification

### Daily Packbox Strategy

The original WeatherMule design considered maintaining a special "current" packbox file that would receive all incoming weather packets.

Further analysis showed that packet storage can be naturally partitioned using the weather station's `dateutc` value.

Example:

```text
dateutc=2026-08-15+22:18:21
```

The date portion is extracted:

```text
2026-08-15
```

and used to generate the packbox filename:

```text
2026-08-15.box
```

Benefits:

- No special "current box" logic required
- Packets automatically group by observation date
- Easier recovery after multi-day internet outages
- Simpler upload processing
- Reduced bookkeeping code

If connectivity is lost for several days:

```text
2026-08-14.box
2026-08-15.box
2026-08-16.box
2026-08-17.box
```

each day's observations accumulate in a separate packbox.

During recovery, WeatherMule can process packboxes chronologically, forwarding the oldest stored observations first and continuing until all backlog data has been transmitted.

This approach allows storage organization to be derived directly from the weather station timestamps rather than requiring WeatherMule to maintain additional tracking state.

### Offline Recovery Implications

Because every weather packet already includes a timestamp supplied by the weather station, WeatherMule can reconstruct historical transmission gaps after internet service is restored.

Example:

```text
Last successful cloud update:
2026-08-14 10:15:37

Current date:
2026-08-17
```

WeatherMule can determine which daily packboxes contain observations that have not yet been forwarded and process them in order.

This design may also support future reconciliation logic comparing locally stored observations with records already present in Ambient Weather services.

### Startup Device Identification

Startup diagnostics were enhanced to display additional network identification information.

Example startup output:

```text
Initializing SD card...
SD card initialized.
SSID: WiFi-IOT
IP: 192.168.1.1
Mac: 01:a6:b7:02:14:29
```

The MAC address was originally considered for use as a request header that could assist with device authentication.

Subsequent network discussions suggested that this may be unnecessary because networking equipment already identifies devices by MAC address at Layer 2.

Even if not used directly for authentication, including the MAC address in startup diagnostics provides several operational benefits:

- Network troubleshooting
- Verifying device identity
- DHCP reservation setup
- Router configuration
- Distinguishing multiple WeatherMule deployments

This became especially relevant after deployment planning for the dedicated IoT wireless network in Silver City.

Future deployment procedures may include assigning static DHCP reservations based on the WeatherMule MAC address so that router reboots do not require an onsite visit to determine a new IP address.

### Operational Principle

A mule carrying weather packets should always know:

```text
Who it is
Where it is
What day its cargo belongs to
```

The MAC address answers the first question.

The IP address answers the second.

Daily packboxes answer the third.

--- 

## Documentation TODO

### Required Libraries

WeatherMule should maintain a list of required Arduino libraries so
new users can compile the project without hunting for dependencies.

Current libraries:

- WiFiNINA
- WiFiUdp
- SD (planned)
- ArduinoJson (planned for current.json, if used)

Future contributors should be able to install all required libraries
before opening WeatherMule.ino.

Consider adding a "Dependencies" section to quick-start.md.

---

## Project Motto

The weather does not stop because the internet is down.

Store the weather first.

Figure everything else out later.
