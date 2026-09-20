## AI WeatherMule Workflow

### Purpose

This document defines the operational contract between WeatherMule and PackStationWeather.

An identical copy of this document should exist in both repositories:

- WeatherMule
- PackStationWeather

This document is intended primarily as AI context so future development, troubleshooting, documentation, and code generation remain consistent across both projects.

If implementation details conflict with this document, stop and resolve the conflict rather than silently inventing alternative behavior.

---

## Core Principle

Save the weather first. Figure everything else out later.

WeatherMule exists to preserve weather observations in environments where weather collection is more reliable than internet connectivity.

The first responsibility is always to save observations.

Delivery and synchronization are secondary concerns.

Lost weather cannot be recreated.

---

## AI Implementation Contract

The following rules are non-negotiable unless explicitly redesigned.

- MUST save every observation before attempting any network operation.
- MUST preserve the original weather station request exactly as received.
- MUST store observations in append-only daily `.box` files.
- MUST treat `.box` files as the local source of truth.
- MUST store session and refresh tokens only in memory.
- MUST NOT persist session tokens to SD card.
- MUST NOT persist refresh tokens to SD card.
- MUST authenticate using session tokens whenever possible.
- MUST renew expired sessions using refresh tokens.
- MUST fall back to API key authentication when necessary.
- MUST prioritize live weather over all historical synchronization activity.
- MUST immediately abandon historical processing when a new observation arrives.
- MUST discard all cached hole information when a new observation arrives.
- MUST ask PackStationWeather again what data is missing after processing a live observation.
- MUST treat missing ranges returned by PackStationWeather as authoritative.
- MUST NOT determine what observations are missing.
- MUST NOT maintain a persistent upload queue.
- MUST NOT maintain a persistent synchronization cursor.
- MUST NOT maintain a persistent file offset.
- MUST NOT maintain historical upload checkpoints.
- MUST distinguish network failures from HTTP responses.
- MUST stop delivery attempts after network failures.
- MUST wait for the next live observation before attempting delivery again.
- MUST remain recoverable after power loss.
- MUST not depend on continuous connectivity.

---

## System Roles

### Weather Station

Creates weather.

### WeatherMule

Preserves weather.

### Packboxes

Protect weather.

### PackStationWeather

Determines what weather is missing.

---

## Silver City Operating Environment

WeatherMule was designed specifically for environments like Silver City.

Typical operation:

```text
06:00 Network available
21:00 Network disabled
```

The network may be unavailable nightly to conserve power.

During winter:

- Solar charging may be limited.
- The network may operate only one or two hours per day.
- Storms may interrupt connectivity for days or weeks.
- A wrangler may need to ski or snowshoe in to restore service.

WeatherMule continues collecting weather regardless of connectivity.

The weather does not stop because the Internet is unavailable.

---

## Time Handling Rules

Humans think in local time.

WeatherMule uses UTC.

PackStationWeather uses UTC.

The weather station's `dateutc` field is authoritative.

The following MUST always use UTC:

- Observation timestamps
- Missing ranges
- Session timestamps
- Upload timestamps
- API timestamps

Summary:

```text
Humans think local.
WeatherMule thinks UTC.
PackStationWeather thinks UTC.
Weather station timestamps are authoritative.
```

---

## Storage Model

### Daily Packboxes

Examples:

```text
20260829.box
20260830.box
20260831.box
```

Characteristics:

- Append-only
- Stored on SD card
- Permanent archive
- Local source of truth
- Historical synchronization source

Every observation MUST be written to the appropriate packbox before any network activity.

### Raw Observation Storage

WeatherMule stores the original request exactly as received.

Example:

```text
GET /data/report/&PASSKEY=...&dateutc=2026-08-31+13:05:00&tempf=72.5 HTTP/1.1
```

WeatherMule never reconstructs weather observations.

Lost observations cannot be recreated.

---

## Parsed Observation Format

Raw observations are converted into name/value pairs before API delivery.

Example:

```json
[
  {
    "FieldName": "tempf",
    "FieldValue": "72.5"
  },
  {
    "FieldName": "humidity",
    "FieldValue": "58"
  }
]
```

The raw request remains stored regardless of delivery success.

---

## Authentication Model

### Permanent API Key

Every mule has a permanent API key.

The API key:

- Survives reboot
- Survives power loss
- Survives token expiration
- Identifies the mule

The API key is the root credential.

### Session Token

Session tokens:

- Stored in memory only
- Used for authenticated API requests
- Never persisted

### Refresh Token

Refresh tokens:

- Stored in memory only
- Used only with `api/checkin`
- Never persisted

### Initial Check-In

When no session token exists:

```text
Authorization: Bearer <ApiKey>
```

Request:

```text
POST api/checkin
```

Response:

```json
{
  "sessionToken": "...",
  "refreshToken": "..."
}
```

Store both tokens in memory.

### Session Renewal

If PackStationWeather returns:

```text
401 Unauthorized
```

Then:

```text
Authorization: Bearer <RefreshToken>
POST api/checkin
```

Receive replacement tokens.

Retry the interrupted operation.

### Refresh Failure

If refresh token renewal fails:

```text
Discard tokens.
Authenticate with API key.
Receive new tokens.
Retry operation.
```

### Power Loss

Power loss destroys:

```text
SessionToken
RefreshToken
```

Power loss does NOT destroy:

```text
ApiKey
Packboxes
```

Upon restart:

```text
Authenticate with ApiKey.
Get new tokens.
Continue.
```

---

## Main Loop Philosophy

WeatherMule is effectively single threaded.

Pseudo-code:

```text
Loop forever

    If weather is waiting
        ProcessIncomingWeather()
        Continue

    ProcessOneBackfillObservation()
```

Fresh weather always has priority.

Historical synchronization only occurs when no live observation is waiting.

---

## Live Observation Workflow

```text
Receive weather request
        |
        V
Save raw observation to packbox
        |
        V
Ensure authenticated
        |
        V
Convert observation to JSON
        |
        V
POST api/unload
        |
        V
Receive hole list
        |
        V
Replace current hole list
```

When a new observation arrives:

```text
Forget the entire Hole-Kit-and-Caboodle.
```

The previous hole list is discarded.

PackStationWeather will provide a fresh authoritative hole list.

---

## Hole Definition

A hole is a missing range of observations for a specific weather station.

Example:

```json
[
  {
    "vendorDeviceKey": "B8BF967E5C9153DF3A2FA5ECC3A5841B",
    "fromUtc": "2026-08-28T12:00:15",
    "toUtc": "2026-08-28T23:59:15"
  }
]
```

Properties:

```text
vendorDeviceKey
    Weather station identifier.
    Equivalent to PASSKEY.

fromUtc
    Beginning of missing range.

toUtc
    End of missing range.
```

A hole is NOT a list of observations.

A hole represents a missing time range.

Multiple holes may exist:

- Different missing ranges for one station.
- Different missing ranges for multiple stations.

---

## Historical Backfill Workflow

WeatherMule stores only holes.

WeatherMule does NOT queue observations.

The packboxes already contain the observations.

Pseudo-code:

```text
Find oldest hole by FromUtc

Determine packbox containing FromUtc

Read forward from top of file

Find first observation where:

PASSKEY == VendorDeviceKey

and

dateutc >= FromUtc

and

dateutc <= ToUtc

Send observation

Advance hole FromUtc forward

Return immediately
```

Only one historical observation is processed per call.

---

## Hole Progress Tracking

The hole itself acts as the progress tracker.

Example:

Initial hole:

```text
FromUtc = 12:00
ToUtc   = 23:59
```

Observation found:

```text
12:05
```

Upload observation.

Advance:

```text
FromUtc = 12:05:01
```

Next pass:

```text
Find next observation after 12:05
```

No separate cursor is required.

No file position is required.

No upload checkpoint is required.

---

## Multiple Weather Stations

A mule may carry weather for multiple stations.

All observations may exist in the same packbox.

Example:

```text
08:00 Station A
08:05 Station B
08:10 Station A
08:15 Station B
```

Stations are distinguished by:

```text
PASSKEY
```

which corresponds to:

```text
vendorDeviceKey
```

in hole responses.

When multiple holes exist:

```text
Process the hole with the oldest FromUtc.
```

As individual holes advance, WeatherMule naturally shifts work between stations.

No special scheduling logic is required.

---

## Hole Completion

When:

```text
FromUtc >= ToUtc
```

the hole is complete.

Remove the hole.

Continue with the next oldest hole.

When no holes remain:

```text
Wait for the next live observation.
```

---

## New Weather Interrupts Everything

If a new observation arrives while processing holes:

```text
Stop historical work.
Discard all hole state.
Save observation.
Send observation.
Receive fresh holes.
Continue.
```

WeatherMule never attempts to remember where it stopped.

PackStationWeather already knows.

---

## Network Failure Handling

Examples:

- DNS failure
- WiFi outage
- Router outage
- Internet outage
- TLS failure
- Timeout

Response:

```text
Keep weather in packboxes.
Stop delivery attempts.
Wait for next observation.
Try again later.
```

No special queue is required.

---

## Synchronization Authority

PackStationWeather owns synchronization knowledge.

WeatherMule owns weather cargo.

Conversation:

```text
WeatherMule:
    Here is the latest observation.

PackStationWeather:
    Thank you.
    Here is what I am missing.

WeatherMule:
    I will haul what I can while the trail is open.
```

WeatherMule never cares why data is missing.

WeatherMule only cares what PackStationWeather needs.

---

## State Recovery

WeatherMule must be recoverable using only:

- API Key
- SD card
- Packboxes
- New observations
- Hole ranges returned by PackStationWeather

Recovery must NOT require:

- Stored session tokens
- Stored refresh tokens
- Upload queues
- Synchronization checkpoints
- File offsets
- Backfill cursors

---

## Final Mental Model

```text
The weather station creates weather.

WeatherMule preserves weather.

Packboxes protect weather.

PackStationWeather determines what is missing.

Holes are temporary work orders.

The hole itself tracks progress.

Fresh weather always comes first.

The mule may forget the entire Hole-Kit-and-Caboodle at any moment.

PackStationWeather remembers.
The mule hauls cargo.

The network may disappear.

The mule does not panic.

The weather remains safe.
```

## Final Rule

```text
The mule does not care why the cargo was not delivered.

The mule only cares whether the cargo was delivered.
```