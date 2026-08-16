# WeatherMule Design

## Overview

WeatherMule is a store-and-forward weather relay designed for locations where weather observations are more reliable than internet connectivity.

The primary objective is simple:

```text
Never lose weather observations.
```

WeatherMule receives weather data from an Ambient Weather WS2000, stores observations locally on SD card, maintains current weather statistics, and attempts to synchronize missing observations when connectivity allows.

---

# High-Level Architecture

```text
                 +----------------+
                 | Ambient Weather|
                 +--------+-------+
                          ^
                          |
                          |
                 +--------+-------+
                 |  WeatherMule   |
                 | Arduino Nano   |
                 +--------+-------+
                          ^
                          |
                          |
                 +--------+-------+
                 |     WS2000     |
                 +----------------+
```

The weather station transmits observations to both Ambient Weather and WeatherMule.

WeatherMule acts as a passive observer and backup delivery service.

---

# Core Design Principles

## Preserve Data First

Weather observations are more valuable than network status.

When in doubt:

```text
Save the observation.
```

Synchronization can happen later.

Lost observations cannot be recovered.

---

## Store Facts

WeatherMule stores the original weather observation exactly as received.

Example:

```text
GET /data/report/&PASSKEY=...
```

Derived values can always be recalculated later.

Original observations cannot.

---

## Simplicity Over Cleverness

WeatherMule is intended to operate:

- unattended
- off-grid
- on limited power
- in remote locations

Simple systems are easier to recover and maintain.

---

# Storage Model

WeatherMule uses two storage types.

## Packboxes (.box)

Packboxes contain the permanent weather archive.

Example:

```text
20260815.box
20260816.box
20260817.box
```

Each packbox contains observations received during a single day.

Example contents:

```text
GET /data/report/&PASSKEY=...
GET /data/report/&PASSKEY=...
GET /data/report/&PASSKEY=...
```

Packboxes are append-only.

Packboxes are the source of truth.

---

## Wrangler's Notebook (current.json)

current.json stores operational state.

Example:

```json
{
  "CurrentPackbox": "20260815.box",

  "LatestLocalUtc": "2026-08-15T22:18:21Z",
  "LatestAmbientUtc": "2026-08-15T17:58:21Z",

  "DailyLowTempF": 58.7,
  "DailyHighTempF": 90.9,

  "MaxWindGust": 15.88
}
```

This file contains:

- synchronization status
- current statistics
- upload progress
- current packbox information

Unlike packboxes, current.json may be overwritten frequently.

---

# WeatherRequest

WeatherMule receives weather observations using HTTP requests.

The WeatherRequest class:

```cpp
WeatherRequest
```

provides:

```text
Raw Request
+
Parsed Parameters
```

Example:

```text
GET /data/report/&PASSKEY=...
```

becomes:

```text
PASSKEY
dateutc
tempf
humidity
windspeedmph
...
```

The raw request is used for storage and replay.

The parsed values are used for statistics and reporting.

---

# Statistics

WeatherMule maintains a running summary of weather conditions.

Examples:

```text
Daily Low Temperature
Daily High Temperature

Maximum Wind Gust

Latest Observation

Rainfall Totals
```

Statistics are updated as observations are received.

Historical packboxes never need to be rescanned during normal operation.

---

# Synchronization Strategy

Traditional systems check:

```text
Is the network available?
```

WeatherMule attempts to answer:

```text
Is Ambient Weather missing observations?
```

Example:

```text
Ambient Latest:
17:58 UTC

WeatherMule Latest:
22:18 UTC
```

Result:

```text
Ambient is behind.
```

WeatherMule should determine which observations are missing and attempt synchronization.

The reason for the outage is irrelevant.

Possible causes include:

- Internet outage
- Ambient outage
- Router outage
- DNS failure
- Power interruption

WeatherMule cares only about whether the cargo was delivered.

---

# Timestamp Authority

WS2000 observations include:

```text
dateutc=YYYY-MM-DD HH:MM:SS
```

WeatherMule treats the weather station timestamp as authoritative for the observation.

This avoids dependence on local clocks and external time services.

---

# Future Work

Planned areas of investigation:

## Ambient Synchronization

Determine:

- Ambient upload destination
- Ambient response behavior
- Historical upload support
- Duplicate handling behavior

## Packbox Recovery

Determine optimal strategy for:

- locating missing observations
- replaying missed observations
- recovering after power loss

## Statistics

Expand locally maintained statistics while preserving simplicity.

---

# Design Vocabulary

WeatherMule uses a pack-train metaphor throughout the project.

| Term | Meaning |
|--------|--------|
| Cargo | Weather observations |
| Packbox | Daily `.box` file |
| Cargo Manifest | WeatherRequest |
| Wrangler's Notebook | current.json |
| Mule Train | Collection of packboxes |
| Delivery | Synchronization with Ambient |

Cute = Bad

Meaningful = Good

Cute + Meaningful = Priceless.

---

# Guiding Principle

The mule does not care why the cargo was not delivered.

The mule only cares whether the cargo was delivered.