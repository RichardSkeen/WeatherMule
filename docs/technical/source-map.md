- WeatherMule.ino
  - Application entry point
  - Performs startup
  - Runs the main processing loop
  - Prioritizes observations over hole processing

- WeatherStationRoutines.h
  - Receives weather station requests
  - Creates WeatherRequest objects
  - Stores observations
  - Uploads observations
  - Returns HTTP responses

- WeatherRequest.h
  - Parses weather station requests
  - Extracts key/value parameters
  - Provides access to observation values
  - Serializes observations for API communication

- StorageRoutines.h
  - Manages Packboxes
  - Appends observations
  - Locates historical observations
  - Supports hole recovery

- ApiClientRoutines.h
  - Performs authentication
  - Maintains Session Tokens
  - Maintains Refresh Tokens
  - Uploads observations
  - Reports Not Observed conditions
  - Retrieves holes

- HoleRoutines.h
  - Maintains active holes
  - Processes oldest hole
  - Coordinates replay operations
  - Removes completed holes

- Hole.h
  - Represents a recovery hole
  - Stores replay state
  - Supports hole serialization

- ApiResponse.h
  - Parses PackStationWeather responses
  - Extracts properties
  - Parses arrays and objects

- SerialMonitorRoutines.h
  - Diagnostic logging
  - Design mode output
  - Utility formatting methods

- KeyValuePair.h
  - Name/value storage object
  - Used by request and response parsing

- Version.h
  - Stores application version information

- weathermule-secrets.h
  - WiFi configuration
  - API host configuration
  - API authentication keys