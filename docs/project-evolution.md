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
PASSKEY=B8BF967...
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

## Project Motto

The weather does not stop because the internet is down.

Store the weather first.

Figure everything else out later.