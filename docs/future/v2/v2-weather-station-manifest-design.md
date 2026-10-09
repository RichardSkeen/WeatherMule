# Weather Station Manifest Design

## Status

Future WeatherMule Version 2 design.

This document records a proposed manifest-driven design for supporting multiple weather stations. It is intentionally separated from the current implementation so work can remain focused on bringing PackStationWeather and the first WeatherMule deployment online.

## Purpose

WeatherMule should be able to receive, preserve, upload, and recover observations from multiple weather stations without embedding each station's field names or request format in the firmware.

The firmware should understand the stable manifest layout. PackStationWeather should own the current manifest contents.

## Current Limitation

The current WeatherRequest implementation contains weather-station-specific knowledge, including:

- The request recognition pattern.
- The device-key field name.
- The observation-date field name.
- Ambient-specific date normalization, including replacing `+` with `T`.

This works for the current Ambient station but requires firmware changes when another station uses different field names or request patterns.

## Design Principle

The manifest describes how WeatherRequest identifies and parses an observation. It does not contain vendor-specific processing instructions.

Every manifest property must have the same meaning for every supported weather station.

WeatherMule does not need to understand the observation's timezone or reinterpret the station's date format. It preserves the raw date value and the original request. PackStationWeather remains responsible for understanding what those values mean.

## Manifest File

The active manifest is stored on the WeatherMule SD card as:

```text
WeatherStationManifest.json
```

The file contains a JSON array with one entry for each weather station request format assigned to the mule.

Initial proposed layout:

```json
[
  {
    "RecognitionPattern": "data/report/",
    "DeviceKeyName": "PASSKEY",
    "DateFieldName": "dateutc"
  }
]
```

The property names may be refined during implementation, but their meanings should remain universal.

### RecognitionPattern

A string used to determine which manifest entry applies to an incoming raw request.

### DeviceKeyName

The name of the request field containing the weather station's device key.

### DateFieldName

The name of the request field containing the observation date exactly as supplied by the weather station.

## Request Processing

When WeatherMule receives a raw request:

1. It finds the manifest entry whose `RecognitionPattern` matches the request.
2. It creates a WeatherRequest using the raw request and the matching manifest entry.
3. WeatherRequest performs the remaining parsing and observation work.

Conceptually:

```cpp
WeatherStationManifest manifest = FindManifest(rawRequest);
WeatherRequest weatherRequest(rawRequest, manifest);
```

WeatherMule does not separately extract the device key, date, or key/value pairs.

## WeatherRequest Responsibilities

Given the raw request and the selected manifest, WeatherRequest is responsible for:

- Preserving the original raw request.
- Building the request's KeyValuePair collection.
- Extracting the device key using `DeviceKeyName`.
- Extracting and preserving `ObservationDateRaw` using `DateFieldName`.
- Producing the observation payload sent to PackStationWeather.
- Providing the information required to select the proper Packbox.
- Providing the information required for observation replay and hole processing.

The rest of WeatherMule continues to operate on WeatherRequest rather than depending directly on manifest field names.

## Observation Date

WeatherRequest adds an `ObservationDateRaw` property containing the date string exactly as received from the weather station.

WeatherMule does not:

- Replace `+` with `T`.
- Append `Z` or an offset.
- Determine the weather station's timezone.
- Convert the value to UTC.
- Convert the value to the PackStationWeather storage timezone.

PackStationWeather owns date interpretation and conversion because it has the weather-station configuration and timezone information.

## Packbox Processing

WeatherRequest supplies the information needed to place the observation into the correct Packbox.

Packboxes continue to preserve the original observation and remain the WeatherMule source of truth for recovery. The manifest changes how WeatherRequest discovers identifying values; it does not change the store-before-delivery principle.

The final Packbox naming rule for multiple stations will be determined during Version 2 implementation. The rule must prevent observations from different stations from being confused while preserving simple append-only storage and replay.

## Hole Processing

Hole processing continues to operate through WeatherRequest objects.

When an observation is read from a Packbox during recovery:

1. The stored raw request is matched to its manifest entry.
2. A WeatherRequest is created from the raw request and manifest.
3. WeatherRequest rebuilds its KeyValuePairs and identifying properties.
4. Existing hole-fill processing uploads the reconstructed observation.

WeatherMule does not need timezone knowledge to replay the original observation.

## Manifest Creation

A PackStationWeather user creates or assigns weather stations to a WeatherMule and uses the WeatherStationKeys function to generate the complete `WeatherStationManifest.json` file.

The initial manifest can be copied onto the WeatherMule SD card during provisioning.

The manifest is a complete replacement document, not a collection of incremental add, update, or delete instructions.

## Manifest Updates from PackStationWeather

PackStationWeather may return a manifest array during check-in.

Conceptual response:

```json
{
  "sessionToken": "...",
  "refreshToken": "...",
  "manifest": []
}
```

Update rule:

- An empty manifest array means the mule keeps its current local manifest.
- A non-empty manifest array is the complete current manifest and overwrites the local `WeatherStationManifest.json` file.

The mule does not compare entries, merge changes, add a station, remove a station, or determine what changed. PackStationWeather sends the complete manifest, and the mule replaces its local copy.

This supports changes such as:

- A field name changing from `dateutc` to `ObservationDateUtc`.
- A user assigning another weather station to the mule.
- A weather station being removed from the mule.
- A recognition pattern changing.

These are manifest-content changes and do not require firmware changes as long as the manifest layout remains compatible with the firmware.

## Firmware Compatibility Boundary

A firmware update is not required when only manifest values change.

Examples that should not require firmware changes:

- A different `RecognitionPattern`.
- A different `DeviceKeyName`.
- A different `DateFieldName`.
- Adding another station entry.
- Removing a station entry.
- Replacing the entire manifest.

A firmware update may be required when the manifest layout introduces a new concept that the existing WeatherRequest implementation does not understand.

The dividing line is:

> Changing manifest contents is configuration. Changing how manifest contents are used is firmware behavior.

## Responsibility Boundaries

### WeatherMule

- Loads the local manifest.
- Receives raw weather-station requests.
- Selects the matching manifest entry by `RecognitionPattern`.
- Passes the raw request and manifest to WeatherRequest.
- Stores the resulting observation.
- Uploads the resulting observation.
- Processes holes using reconstructed WeatherRequest objects.
- Replaces the local manifest when PackStationWeather supplies a non-empty complete manifest.

### WeatherRequest

- Interprets the raw request using the selected manifest.
- Builds KeyValuePairs.
- Preserves the original request.
- Extracts the device key.
- Preserves `ObservationDateRaw`.
- Provides the consistent observation interface used by storage, upload, and hole recovery.

### PackStationWeather

- Owns weather-station configuration.
- Generates the complete manifest assigned to a mule.
- Sends manifest replacements during check-in when appropriate.
- Interprets raw observation dates using the station's configuration.
- Performs timezone and storage-time conversions.
- Remains the authority for identifying holes.

## Expected Benefits

- One WeatherMule can carry observations for multiple weather stations.
- New stations can be assigned without merging configuration on the mule.
- Field-name and recognition-pattern changes can be deployed without reflashing firmware.
- WeatherRequest remains the single request-parsing boundary.
- WeatherMule remains independent of vendor-specific timezone and date quirks.
- PackStationWeather owns configuration and interpretation while WeatherMule owns preservation and delivery.
- The existing store-before-delivery and server-authoritative hole-recovery principles remain intact.

## Deferred Implementation Questions

The following details are intentionally deferred until Version 2 implementation:

- The C++ representation of a manifest entry.
- JSON parsing and memory limits for the manifest array.
- Maximum supported manifest entries.
- Behavior when no recognition pattern matches.
- Validation before replacing the current manifest.
- Safe file replacement if power fails during a manifest update.
- The final Packbox naming strategy for multiple weather stations.
- Whether the active manifest is cached in memory, read from SD on demand, or both.
- How PackStationWeather decides when to include a non-empty manifest in a check-in response.
- Compatibility behavior when a manifest requires a newer firmware layout.

## Summary

Version 2 makes WeatherMule manifest-driven without making it vendor-aware.

The mule receives a raw request, selects a manifest using `RecognitionPattern`, and gives both to WeatherRequest. WeatherRequest performs the parsing, preserves the device key and raw observation date, builds the observation, and supplies the information required for Packbox storage and hole recovery.

PackStationWeather owns and distributes the complete manifest. A non-empty manifest received during check-in replaces the mule's local manifest without comparison or merging.
