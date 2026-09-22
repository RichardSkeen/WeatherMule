# Source Map

## Design Ownership

WeatherMule is organized around a small number of focused subsystems. Each major source file owns a specific part of the application's behavior.

- Application flow is implemented by `WeatherMule.ino`.
- Weather request handling is implemented by `WeatherStationRoutines.h`.
- Observation parsing and representation are implemented by `WeatherRequest.h`.
- Storage and replay are implemented by `StorageRoutines.h`.
- Authentication and API communication are implemented by `ApiClientRoutines.h`.
- Hole recovery is implemented by `HoleRoutines.h`.

The files below describe the primary responsibilities of each source file.

## Source Files

### WeatherMule.ino.h

- Application entry point
- Performs startup
- Connects to WiFi
- Starts the local HTTP server
- Initializes SD storage
- Runs the main processing loop
- Prioritizes incoming observations over hole processing

*Implementation: [WeatherMule.ino](../../WeatherMule.ino)*

### WeatherStationRoutines.h

- Receives HTTP requests
- Classifies request types
- Creates `WeatherRequest` objects
- Coordinates storage operations
- Coordinates upload operations
- Handles logging control requests
- Returns HTTP responses

*Implementation: [WeatherStationRoutines.h](../../WeatherStationRoutines.h)*

### WeatherRequest.h

- Preserves the original weather station request
- Parses weather station requests
- Identifies weather observation requests
- Extracts HTTP method and version information
- Extracts name/value parameters
- Provides access to observation values
- Serializes observations for API communication
- Normalizes observation timestamps

*Implementation: [WeatherRequest.h](../../WeatherRequest.h)*

### StorageRoutines.h

- Initializes SD card storage
- Manages Packboxes
- Creates daily Packbox filenames
- Appends original observations
- Locates historical observations
- Replays stored observations
- Traverses Packboxes chronologically
- Maintains replay bookmarks
- Supports hole recovery

*Implementation: [StorageRoutines.h](../../StorageRoutines.h)*

### ApiClientRoutines.h

- Performs on-demand authentication
- Maintains Session Tokens
- Maintains Refresh Tokens
- Builds authenticated API requests
- Uploads current observations
- Uploads recovered observations
- Reports Not Observed conditions
- Reads PackStationWeather responses
- Retrieves holes returned by PackStationWeather
- Retries requests after authentication failures

*Implementation: [ApiClientRoutines.h](../../ApiClientRoutines.h)*

### HoleRoutines.h

- Maintains the active hole collection
- Adds holes returned by PackStationWeather
- Selects the oldest hole
- Coordinates replay operations
- Advances hole recovery progress
- Reports holes containing no recorded observations
- Removes completed holes
- Clears and replaces the active hole list

*Implementation: [HoleRoutines.h](../../HoleRoutines.h)*

### Hole.h

- Represents a recovery work order
- Stores the PackStationWeather device key
- Stores known recovery boundaries
- Stores recovery progress
- Stores replay bookmark information
- Determines the initial Packbox filename
- Serializes hole information for API communication

The `FromUtc` and `ToUtc` fields identify observations already known to PackStationWeather. Observations between those boundaries are the recovery candidates.

*Implementation: [Hole.h](../../Hole.h)*

### ApiResponse.h

- Extracts JSON from PackStationWeather responses
- Determines whether a response contains an object or array
- Parses JSON objects
- Parses arrays of JSON objects
- Stores parsed properties
- Provides access to response values

*Implementation: [ApiResponse.h](../../ApiResponse.h)*

### SerialMonitorRoutines.h

- Provides diagnostic logging
- Controls design-mode output
- Logs parsed weather requests
- Formats integer values as hexadecimal strings

*Implementation: [SerialMonitorRoutines.h](../../SerialMonitorRoutines.h)*

### KeyValuePair.h

- Represents a name/value pair
- Stores parsed request parameters
- Stores parsed response properties
- Serializes name/value pairs for API communication

*Implementation: [KeyValuePair.h](../../KeyValuePair.h)*

### Version.h

- Stores WeatherMule application version information
- Provides the version used in the API User-Agent header

*Implementation: [Version.h](../../Version.h)*

### weathermule-secrets.example.h

- Provides the configuration template for device-specific secrets
- Defines the WeatherMule device name
- Defines WiFi credentials
- Defines Ambient Weather credentials
- Defines the PackStationWeather host and port
- Defines the PackStationWeather API key
- Defines PackStationWeather endpoint configuration values

The example file must be copied and populated with deployment-specific values before use. Credentials must not be committed to source control.

*Implementation: [weathermule-secrets.example](../../weathermule-secrets.example)*