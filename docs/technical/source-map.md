# Source Map

## Design Ownership

WeatherMule is organized around a small number of focused subsystems. Each major source file owns a specific part of the application's behavior.

- Application flow is implemented by WeatherMule.ino.
- WiFi connection and weather station server lifecycle are implemented by WiFiRoutines.h.
- Weather request handling is implemented by WeatherStationRoutines.h.
- Observation parsing and representation are implemented by WeatherRequest.h.
- Storage and replay are implemented by StorageRoutines.h.
- Authentication and PackStationWeather API communication are implemented by ApiClientRoutines.h.
- Hole recovery is implemented by HoleRoutines.h.

The files below describe the primary responsibilities of each source file.

## Source Files

### WeatherMule.ino

- Application entry point
- Performs startup
- Initializes SD storage
- Initiates WiFi maintenance
- Defines the weather station HTTP server
- Runs the main processing loop
- Gates network-dependent processing on WiFi availability
- Prioritizes incoming observations over hole processing

_Implementation: [WeatherMule.ino](../../WeatherMule.ino)_

### WiFiRoutines.h

- Monitors WiFi connection state
- Performs the initial WiFi connection attempt
- Attempts immediate reconnection when WiFi loss is detected
- Retries failed WiFi connections at five-minute intervals
- Starts the weather station server when WiFi becomes available
- Stops the weather station server when WiFi is lost
- Maintains WiFi and weather station server lifecycle state

_Implementation: [WiFiRoutines.h](../../WiFiRoutines.h)_

### WeatherStationRoutines.h

- Receives weather station HTTP requests
- Classifies request types
- Creates WeatherRequest objects
- Acknowledges weather observations
- Closes the weather station connection before downstream processing
- Coordinates Packbox storage
- Coordinates PackStationWeather upload
- Handles logging control requests
- Provides HTTP response helpers

_Implementation: [WeatherStationRoutines.h](../../WeatherStationRoutines.h)_

### WeatherRequest.h

- Preserves the original weather station request
- Parses weather station requests
- Identifies weather observation requests
- Extracts HTTP method and version information
- Extracts name/value parameters
- Provides access to observation values
- Serializes observations for API communication
- Provides access to raw `dateutc` values
- Produces normalized UTC timestamps when required

_Implementation: [WeatherRequest.h](../../WeatherRequest.h)_

### StorageRoutines.h

- Initializes SD card storage
- Manages Packboxes
- Creates daily Packbox filenames
- Appends original weather station requests
- Locates historical observations
- Matches observations using raw `dateutc` values
- Replays stored observations
- Traverses Packboxes chronologically
- Continues recovery across Packbox boundaries
- Uses byte offsets to optimize replay searches
- Validates optimized positions against raw timestamps
- Supports hole recovery

_Implementation: [StorageRoutines.h](../../StorageRoutines.h)_

### ApiClientRoutines.h

- Performs PackStationWeather API operations
- Maintains Session Tokens
- Maintains Refresh Tokens
- Performs check-in authentication
- Recovers authentication after `401 Unauthorized`
- Falls back from Refresh Token authentication to the API key
- Builds authenticated API requests
- Uploads current observations
- Uploads recovered observations
- Reports Not Observed conditions
- Reads PackStationWeather responses
- Processes holes returned by PackStationWeather
- Retries API operations after successful authentication recovery

_Implementation: [ApiClientRoutines.h](../../ApiClientRoutines.h)_

### HoleRoutines.h

- Maintains the active hole collection
- Adds holes returned by PackStationWeather
- Stores normalized and raw hole boundaries
- Selects the oldest hole
- Coordinates replay operations
- Advances normalized and raw recovery boundaries
- Reports holes containing no recorded observations
- Removes completed holes
- Clears and replaces the active hole list

_Implementation: [HoleRoutines.h](../../HoleRoutines.h)_

### Hole.h

- Represents a recovery work order
- Stores the PackStationWeather device key
- Stores normalized recovery boundaries in `FromUtc` and `ToUtc`
- Stores Packbox-facing recovery boundaries in `FromDateRaw` and `ToDateRaw`
- Stores partial-recovery state in `PartiallyFilled`
- Stores the replay optimization position in `ByteOffset`
- Determines the initial Packbox filename
- Serializes hole information

`FromUtc` and `ToUtc` identify the normalized hole boundaries.

FromDateRaw and ToDateRaw contain the corresponding raw dateutc values used to locate and compare observations in Packbox storage.

_Implementation: [Hole.h](../../Hole.h)_

### HttpClienus.h

- Performs HTTP communication
- Creates outbound HTTP requests
- Reads HTTP responses
- Supports multi-packet responses
- Parses HTTP status lines
- Detects transport failures
- Applies receive timeouts
- Closes network connections
- Returns HttpResponsum objects

_Implementation: [HttpClienus.h](../../HttpClienus.h)_

### ApiResponse.h

- Extracts JSON from PackStationWeather responses
- Determines whether a response contains an object or array
- Parses JSON objects
- Parses arrays of JSON objects
- Stores parsed properties
- Provides access to response values

_Implementation: [ApiResponse.h](../../ApiResponse.h)_

### SerialMonitorRoutines.h

- Provides diagnostic logging
- Controls design-mode output
- Logs parsed weather requests
- Formats integer values as hexadecimal strings

_Implementation: [SerialMonitorRoutines.h](../../SerialMonitorRoutines.h)_

### KeyValuePair.h

- Represents a name/value pair
- Stores parsed request parameters
- Stores parsed response properties
- Serializes name/value pairs for API communication

_Implementation: [KeyValuePair.h](../../KeyValuePair.h)_

### Version.h

- Stores WeatherMule application version information
- Provides the version used in the API User-Agent header

_Implementation: [Version.h](../../Version.h)_

### weathermule-secrets.example

- Provides the configuration template for device-specific settings and secrets
- Defines the WeatherMule device name
- Defines WiFi credentials
- Defines the PackStationWeather host and port
- Defines the PackStationWeather API key
- Defines PackStationWeather endpoint configuration values

The example file must be copied to:

    weathermule-secrets.h

and populated with deployment-specific values before use.

Credentials must not be committed to source control.

_Implementation: ../../weathermule-secrets.example.h_