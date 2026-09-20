# Purpose

WeatherMule is an embedded store-and-forward relay for weather observations.

Its responsibility is to:

- Receive weather observations from an Ambient Weather station.
- Preserve observations locally on removable storage.
- Forward observations to PackStationWeather.
- Recover from communication outages.
- Synchronize observations that were missed by the server.
- Operate unattended in environments where network connectivity may be unreliable.

WeatherMule treats the original weather observation as the source record and stores observations before attempting remote delivery. Local storage allows observations to be replayed later if PackStationWeather is unavailable.

The system is designed around reliability, recoverability, and low operational complexity rather than real-time delivery guarantees.

# Core Principles

## Preserve Data Before Delivery

Weather observations are stored locally before remote delivery is attempted. Local storage ensures observations remain available even when PackStationWeather, network connectivity, or supporting infrastructure are unavailable.

## Store Original Observations

WeatherMule preserves the original weather station observation as received. The original observation is considered the source record. Derived values and statistics may be recalculated later from the stored observations.

## Stored Original Observations Are the Source of Truth

Packboxes (.box files) serve as the authoritative record of observations retained by WeatherMule. Operational state and synchronization status may be recreated from packbox data if necessary.

## Recovery Is a Normal Operation

Network interruptions, power failures, and service outages are expected operating conditions. The system is designed to identify missing observations and recover delivery through synchronization processes.

## Prefer Simple and Observable Behavior

The system favors straightforward implementations that are easy to understand, troubleshoot, and recover in remote deployments. Simplicity is preferred over optimization unless a measurable benefit exists.

## Operate Unattended

WeatherMule is intended to operate for extended periods without human intervention. Normal processing, storage, delivery, and recovery operations should occur automatically whenever possible.

# System Components

## WeatherMule

WeatherMule is the application entry point.

Responsibilities:

- Connect to WiFi.
- Initialize local storage.
- Accept weather station requests.
- Coordinate upload and synchronization activities.
- Process holes when observations are not being received.

Primary file:

- WeatherMule.ino

## WeatherRequest

WeatherRequest parses incoming weather station requests and exposes observation values as key/value pairs.

Responsibilities:

- Parse incoming HTTP requests.
- Extract weather observation parameters.
- Provide access to individual values.
- Serialize observations for API transport.

Primary file:

- WeatherRequest.h

## Weather Station Interface

The Weather Station Interface receives observations from the weather station and coordinates local storage and delivery.

Responsibilities:

- Receive weather station requests.
- Validate observation requests.
- Store observations locally.
- Upload observations to PackStationWeather.
- Return HTTP responses to the weather station.

Primary file:

- WeatherStationRoutines.h

## Storage Subsystem

The Storage Subsystem manages persistent local storage using packboxes.

Responsibilities:

- Create daily packboxes.
- Append observations.
- Locate observations for synchronization.
- Read observations from historical packboxes.
- Maintain recovery data required for hole filling.

Primary file:

- StorageRoutines.h

## API Client

The API Client manages communication with PackStationWeather.

Responsibilities:

- Authenticate with PackStationWeather.
- Maintain session and refresh tokens.
- Upload observations.
- Report observations that cannot be recovered.
- Retrieve hole information.

Primary file:

- ApiClientRoutines.h

## Hole Processor

The Hole Processor recovers observations that are missing from PackStationWeather.

Responsibilities:

- Track holes returned by the server.
- Locate missing observations in packboxes.
- Upload recovered observations.
- Report observations that can no longer be located.
- Remove completed holes from processing.

Primary file:

- HoleRoutines.h

## API Response Parser

The API Response Parser provides lightweight JSON parsing for responses returned by PackStationWeather.

Responsibilities:

- Parse API responses.
- Extract property values.
- Convert JSON arrays and objects into navigable structures.

Primary file:

- ApiResponse.h

# Packboxes

Packboxes are the persistent storage mechanism used by WeatherMule.

A packbox is a daily append-only file containing the original weather station observations received during a single UTC date. Packboxes provide the durable record used for recovery, synchronization, and future processing.

## Naming

Packboxes are named using the observation date.

Examples:

2026-08-15.box
2026-08-16.box
2026-08-17.box

The date portion is derived from the observation timestamp rather than the device clock.

## Contents

Each line within a packbox contains a complete weather station observation in its original form.

Example:

GET /data/report/&PASSKEY=...

Observations are stored without modification to preserve the original source record.

## Append-Only Design

New observations are always appended to the end of the appropriate packbox.

Existing observations are never modified or removed.

This design:

- minimizes write operations
- simplifies recovery
- reduces the risk of corruption
- preserves original observations

## Source of Truth

Stored observations within packboxes are considered the authoritative record maintained by WeatherMule.

Synchronization, hole filling, and future processing operations are performed using observations recovered from packboxes.

Derived statistics and operational state may be recreated from packbox contents if necessary.

## Synchronization

When PackStationWeather identifies missing observations, WeatherMule locates the corresponding records within the appropriate packbox and replays them to the server.

Packboxes therefore serve as both:

- the local archive
- the recovery mechanism

## Recovery

Packboxes allow WeatherMule to recover from interruptions including:

- network outages
- server outages
- power interruptions
- device restarts

As long as observations have been successfully written to a packbox, they remain available for future delivery attempts.

# Authentication

WeatherMule authenticates with PackStationWeather using a two-stage token process.

## API Key

Each WeatherMule instance is configured with a unique API key.

The API key is a long-lived credential and is typically used only when establishing a new authenticated session.

## Check-In

WeatherMule performs a check-in operation when:

- no Session Token is available
- a Session Token has expired
- a Session Token has been revoked
- no Refresh Token is available
- a Refresh Token has expired
- a Refresh Token has been revoked

A successful check-in returns:

- Session Token
- Refresh Token

These tokens are stored in memory and used for subsequent communication.

## Token Lifecycle

The Session Token is intended for normal API operations and has a relatively short lifetime.

The Refresh Token has a longer lifetime and is used to obtain a replacement Session Token when the current Session Token expires.

During normal operation:

1. WeatherMule authenticates using the API Key.
2. PackStationWeather returns a Session Token and Refresh Token.
3. API requests use the Session Token.
4. When the Session Token expires, WeatherMule performs another check-in using the Refresh Token.
5. PackStationWeather returns a replacement Session Token.
6. Normal operation continues.

The API Key is normally not transmitted again until both token-based mechanisms become unavailable.

## Refresh Token Rotation

Refresh Tokens may be rotated during Session Token renewal.

When a new Refresh Token is issued, it inherits the remaining expiration period of the token it replaces rather than creating a new refresh window.

This approach:

- limits long-term exposure of a Refresh Token
- reduces repeated transmission of the API Key
- preserves the original refresh-token expiration policy
- allows token values to change periodically during normal operation

## Authentication Failure

If token-based authentication fails, WeatherMule falls back to API Key authentication by performing a new check-in.

Authentication failures do not affect local observation storage. Observations continue to be preserved within packboxes and may be replayed after authentication is restored.


# Holes

A hole represents a range of observations that PackStationWeather expects to exist but cannot locate within its observation history.

Holes are identified by PackStationWeather and returned to WeatherMule during normal upload operations.

## Hole Structure

A hole contains:

- Vendor Device Key
- From UTC
- To UTC
- Partially Filled (used only by WeatherMule to track progress)
- Byte Offset (used only by WeatherMule to track progress)

The time range defines the missing observations between FromUtc and ToUtc.

Example:

{
    "vendorDeviceKey": "...",
    "fromUtc": "2026-08-15T17:00:00Z",
    "toUtc": "2026-08-15T18:00:00Z",
    "partiallyFilled": false,
    "byteOffset": 0
}

Primary file:

Hole.h

## Hole Processing

When a hole is received, WeatherMule locates observations associated with the hole and gradually replays missing observations to PackStationWeather.

Hole recovery is performed incrementally rather than as a single batch operation. This allows WeatherMule to remain responsive to new observations while recovery is in progress.

Progress is tracked so recovery can resume efficiently between processing cycles.

Hole processing continues until the boundary identified by ToUtc is reached or until WeatherMule determines that additional observations are unavailable.

Holes are transient recovery instructions rather than permanent state.

Whenever a new observation is successfully uploaded, any unfinished holes are discarded. PackStationWeather returns a new set of holes if any still exist.

This allows PackStationWeather to remain the authority for determining which observations are still missing while keeping WeatherMule focused on recovering only currently relevant gaps.

# Recovery

WeatherMule is designed to recover from temporary interruptions without losing previously stored observations.

Recovery is based on the assumption that communication failures are normal operating conditions rather than exceptional events.

## Recovery Sources

Recovery is performed using observations stored in packboxes.

As long as an observation has been successfully written to a packbox, WeatherMule retains the information required to attempt future delivery.

## Recoverable Conditions

WeatherMule can recover from conditions including:

- network interruptions
- PackStationWeather outages
- authentication failures
- device restarts
- temporary power interruptions

Recovery occurs automatically during normal operation.

## Recovery Process

After communication with PackStationWeather is restored, WeatherMule uploads new observations as they are received.

PackStationWeather identifies any gaps in its observation history and returns the corresponding holes.

WeatherMule uses the holes to locate and replay missing observations from packboxes until the identified gaps have been resolved or until additional observations cannot be located.

## Recovery Authority

PackStationWeather is responsible for determining which observations are missing.

WeatherMule is responsible for locating and replaying observations requested by PackStationWeather.

This separation allows recovery decisions to be made using the server's complete observation history while allowing WeatherMule to focus on preservation and delivery of observations.

## Recovery Limitations

Recovery requires that observations have been successfully stored in a packbox.

Observations that were never received or could not be stored cannot be recovered.

When PackStationWeather requests observations that WeatherMule cannot locate, the condition is reported as Not Observed.


# Design Vocabulary

WeatherMule uses a pack-train metaphor to provide consistent terminology throughout the project.

| Term | Meaning |
|--------|--------|
| WeatherMule | The Arduino-based store-and-forward weather relay |
| Cargo | Weather observations |
| Packbox | A daily `.box` file containing stored observations |
| Mule Train | The complete collection of packboxes |
| PackStationWeather | The server responsible for observation storage, authentication, and hole identification |
| Hole | A range of observations expected by PackStationWeather but not currently available in its observation history |
| Check-In | Authentication request used to obtain Session and Refresh tokens |
| Delivery | Successful upload of an observation to PackStationWeather |
| Replay | Resending a previously stored observation from a packbox |
| Not Observed | A condition indicating WeatherMule cannot locate an observation requested by a hole |

The design vocabulary exists to provide concise terminology for concepts that appear frequently throughout the codebase, documentation, and operational workflows.
