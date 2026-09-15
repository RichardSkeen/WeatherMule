\# WeatherMule Workflow



\## What is WeatherMule?



WeatherMule sits between one or more weather stations and PackStationWeather.



Its job is simple:



1\. Receive weather observations.

2\. Save them immediately to local storage.

3\. Deliver them to PackStationWeather when possible.

4\. Help recover any historical observations that PackStationWeather is missing.



The mule's first responsibility is preserving weather data.



If the Internet disappears, the power fails, or PackStationWeather cannot be reached, the weather must still be safely stored on the SD card.



Weather can always be uploaded later.

Lost weather cannot be recreated.



\---



\# Core Philosophy



The mule follows a simple rule:



> Save first. Deliver second.



Every observation is written to storage before any network activity occurs.



The SD card is the mule's source of truth.



PackStationWeather is the synchronization authority.



The mule remembers weather.



PackStationWeather remembers what weather is missing.



\---



\# Startup Workflow



When the mule powers on:



1\. Connect to WiFi.

2\. Initialize SD card storage.

3\. Start the local HTTP server.

4\. Wait for incoming weather station requests.



No authentication occurs at startup.



Authentication is only performed when the mule needs to communicate with PackStationWeather.



\---



\# Live Observation Workflow



When a weather station submits an observation:



```text

Weather Station

&#x20;       |

&#x20;       V

Receive HTTP Request

&#x20;       |

&#x20;       V

Save Raw Request To Packbox

&#x20;       |

&#x20;       V

Upload Observation

&#x20;       |

&#x20;       V

Receive Hole List

&#x20;       |

&#x20;       V

Replace Existing Holes

```



\## Save First



The original weather station request is stored exactly as received.



Example:



```http

GET /data/report/\&PASSKEY=...\&tempf=72.5 HTTP/1.1

```



The mule does not modify or reconstruct observations before storage.



The original request remains permanently archived in a daily packbox file.



\---



\## Upload Latest Observation



After storage succeeds:



1\. Parse the observation into name/value pairs.

2\. Convert it into JSON.

3\. Deliver it to PackStationWeather.



Example:



```json

\[

&#x20; {

&#x20;   "FieldName": "tempf",

&#x20;   "FieldValue": "72.5"

&#x20; }

]

```



The mule does not need to understand sensors.



It simply hauls whatever cargo the weather station provides.



Unknown sensors are allowed.



Future sensors are allowed.



The mule carries sacks.

It does not inspect sacks.



\---



\# Authentication Workflow



WeatherMule uses three credentials:



\## API Key



Permanent credential configured in secrets.



Used only when:



\- First boot

\- Session recovery

\- Refresh failure



The API key survives reboot and power loss.



\---



\## Session Token



Obtained from PackStationWeather.



Used for normal authenticated operations.



Stored only in memory.



Never written to SD card.



\---



\## Refresh Token



Obtained from PackStationWeather.



Used to renew expired sessions.



Stored only in memory.



Never written to SD card.



\---



\# Check-In Workflow



If no session token exists:



```text

Use API Key

&#x20;       |

&#x20;       V

/api/checkin

&#x20;       |

&#x20;       V

Receive Session Token

Receive Refresh Token

```



Tokens remain in memory until:



\- Reboot

\- Power loss

\- Session replacement



\---



\# Session Recovery



If PackStationWeather returns:



```text

401 Unauthorized

```



The mule:



1\. Tries to check in using the refresh token.

2\. Receives replacement tokens.

3\. Retries the failed operation.



If refresh authentication fails:



1\. Discard tokens.

2\. Authenticate using the API key.

3\. Receive new tokens.

4\. Retry the operation.



\---



\# Hole Workflow



After a successful upload, PackStationWeather responds with a list of missing ranges.



These ranges are called holes.



Example:



```json

\[

&#x20; {

&#x20;   "vendorDeviceKey": "ABC123",

&#x20;   "fromUtc": "2026-09-01T00:00:00",

&#x20;   "toUtc": "2026-09-01T12:00:00"

&#x20; }

]

```



A hole means:



> PackStationWeather is missing weather between these two timestamps.



A hole does not contain weather.



A hole is simply a work order.



\---



\# Replacing Hole Lists



Whenever a live observation is uploaded:



```text

Discard Existing Holes

&#x20;       |

&#x20;       V

Store New Holes

```



The mule does not merge hole lists.



The mule does not attempt to calculate what is missing.



PackStationWeather is the authority.



If PackStationWeather says a hole exists, the hole exists.



If PackStationWeather no longer reports a hole, the mule forgets it.



\---



\# Historical Backfill Workflow



Historical synchronization only occurs when no live weather is waiting.



Fresh observations always have priority.



Expected workflow:



```text

No Live Weather

&#x20;       |

&#x20;       V

Select Oldest Hole

&#x20;       |

&#x20;       V

Find Matching Packbox

&#x20;       |

&#x20;       V

Locate Observation

&#x20;       |

&#x20;       V

Upload One Observation

&#x20;       |

&#x20;       V

Advance Hole

&#x20;       |

&#x20;       V

Return To Main Loop

```



Only one historical observation should be processed at a time.



This keeps the mule responsive to new weather arrivals.



\---



\# Hole Progress Tracking



The hole itself serves as the progress tracker.



Example:



Initial hole:



```text

FromUtc = 12:00

ToUtc   = 23:59

```



Observation delivered:



```text

12:05

```



Advance hole:



```text

FromUtc = 12:05:01

```



On the next pass, work resumes from the updated timestamp.



No separate cursor is required.



No file offsets are required.



No upload queue is required.



\---



\# New Weather Always Wins



If a new observation arrives while historical processing is underway:



```text

Stop Historical Work

&#x20;       |

&#x20;       V

Save New Observation

&#x20;       |

&#x20;       V

Upload New Observation

&#x20;       |

&#x20;       V

Receive New Holes

```



The mule is allowed to forget all previous hole state.



PackStationWeather already knows what is missing.



\---



\# Network Failure Handling



Network failures include:



\- WiFi outage

\- Router outage

\- DNS failure

\- Server unavailable

\- Timeout

\- TLS failure



When network operations fail:



```text

Keep Weather In Packboxes

&#x20;       |

&#x20;       V

Stop Delivery Attempts

&#x20;       |

&#x20;       V

Wait For Next Observation

```



Nothing is removed from storage.



Nothing is lost.



The mule simply waits for another opportunity to haul cargo.



\---



\# Storage Model



Weather is stored in daily packboxes.



Example:



```text

20260908.box

20260909.box

20260910.box

```



Characteristics:



\- Append only

\- Permanent archive

\- Local source of truth

\- Historical synchronization source



Every observation must be written before any delivery attempt occurs.



\---



\# Multiple Weather Stations



A mule may carry weather for multiple stations.



Stations are identified by:



```text

PASSKEY

```



which corresponds to:



```text

vendorDeviceKey

```



in hole responses.



A single packbox may contain observations from many stations.



Backfill processing uses the vendor device key to locate the correct observations.



\---



\# Recovery After Power Loss



After power loss the mule loses:



\- Session token

\- Refresh token



The mule retains:



\- API key

\- SD card

\- Packboxes

\- Historical weather



Upon restart:



1\. Reconnect to WiFi.

2\. Initialize storage.

3\. Wait for weather.

4\. Authenticate when needed using the API key.

5\. Continue normal operation.



No manual recovery steps should be required.



\---



\# Mental Model



The weather station creates weather.



WeatherMule preserves weather.



Packboxes protect weather.



PackStationWeather determines what is missing.



Holes are temporary work orders.



Fresh weather always comes first.



The mule may forget every hole at any moment.



PackStationWeather remembers.



The mule just hauls cargo.

