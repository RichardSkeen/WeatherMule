# Protocol Discovery

This document records discoveries made while investigating the Ambient Weather WS2000 communication protocol.

Unlike the design documentation, this document records observations and evidence rather than implementation decisions.

---

# Current Status

## Hardware

- Ambient Weather WS2000
- Arduino Nano 33 IoT
- Arduino Nano Connector Carrier

## Communication Method

The WS2000 is configured to use the Ambient Weather Custom Server feature.

The Arduino hosts a web server and receives weather observations using HTTP requests.

---

# Captured Request

Example:

```text
GET /data/report/&PASSKEY=4b7d92e1c8runningtothebarnPHONYKEY5f3e8a1c
&stationtype=AMBWeatherPro_V5.1.5
&dateutc=2026-08-15+22:18:21
&tempf=90.9
&humidity=27
&windspeedmph=4.25
&windgustmph=5.82
&maxdailygust=15.88
&winddir=12
&winddir_avg10m=319
&uv=7
&solarradiation=741.78
&hourlyrainin=0.000
&eventrainin=0.000
&dailyrainin=0.000
&weeklyrainin=0.000
&monthlyrainin=0.000
&yearlyrainin=18.583
&battout=1
&tempinf=79.3
&humidityin=43
&baromrelin=28.939
&baromabsin=28.939
&battin=1
HTTP/1.1
```

---

# Identified Fields

## Station Information

| Field | Example |
|---------|---------|
| PASSKEY | PHONYKEY5f3e... |
| stationtype | AMBWeatherPro_V5.1.5 |

## Time

| Field | Example |
|---------|---------|
| dateutc | 2026-08-15+22:18:21 |

Observation:

`dateutc` is transmitted in UTC (Zulu) time.

---

## Outdoor Conditions

| Field | Description |
|---------|---------|
| tempf | Outdoor temperature |
| humidity | Outdoor humidity |
| windspeedmph | Wind speed |
| windgustmph | Wind gust |
| maxdailygust | Maximum daily gust |
| winddir | Current wind direction |
| winddir_avg10m | Average wind direction |
| uv | UV index |
| solarradiation | Solar radiation |

---

## Rainfall

| Field | Description |
|---------|---------|
| hourlyrainin | Hourly rainfall |
| eventrainin | Event rainfall |
| dailyrainin | Daily rainfall |
| weeklyrainin | Weekly rainfall |
| monthlyrainin | Monthly rainfall |
| yearlyrainin | Yearly rainfall |

---

## Indoor Conditions

| Field | Description |
|---------|---------|
| tempinf | Indoor temperature |
| humidityin | Indoor humidity |

---

## Barometric Pressure

| Field | Description |
|---------|---------|
| baromrelin | Relative pressure |
| baromabsin | Absolute pressure |

---

## Battery Status

| Field | Description |
|---------|---------|
| battout | Outdoor sensor battery |
| battin | Indoor console battery |

---

# Observations

## PASSKEY Remains Constant

Observed across multiple requests.

The value appears to remain stable and does not change between uploads.

Current theory:

- Device identifier
- Station identifier
- Account identifier

No evidence currently suggests the value is calculated per request.

---

## No MAC Address Observed

To date, no MAC address has been observed in captured requests.

An older utility function named:

```cpp
MacToString()
```

exists in earlier prototype code but is currently unused.

Its purpose remains unknown.

---

## Custom Server Behavior

Observed:

```text
WS2000
    |
    +--> Ambient Weather
    |
    +--> Custom Server
```

The Custom Server does not appear to replace Ambient uploads.

Both appear to receive observations simultaneously.

---

## Upload Interval

Development configuration:

```text
1 minute
```

Production target:

```text
5 minutes
```

The one-minute interval significantly reduces testing time during development.

---

# Open Questions

## Ambient Upload Host

Unknown.

Need to determine:

- Hostname
- URL
- Port
- Protocol

used by the WS2000 when communicating with Ambient Weather.

---

## Ambient Response

Currently unknown.

Need to determine:

- Success responses
- Error responses
- Retry behavior

---

## Historical Upload Support

Need to determine whether Ambient respects:

```text
dateutc
```

during delayed uploads.

Example:

```text
Observation occurs:
2026-08-15 04:00 UTC

Upload occurs:
2026-08-15 12:00 UTC
```

Questions:

- Is the observation stored at 04:00?
- Is it stored at 12:00?
- Is it rejected?

This question has significant impact on the WeatherMule architecture.

---

# Future Investigation

- Determine Ambient upload destination.
- Capture full HTTP request headers.
- Capture Ambient response.
- Test replay of a previously stored observation.
- Determine whether dateutc is honored.
- Determine duplicate handling behavior.

---

# Protocol Philosophy

Observe first.

Assume nothing.

Store facts.

Derive conclusions later.