# WeatherMule Documentation

This documentation is organized into technical references, project stories, hardware references, and supporting materials.

For readers new to the project, follow the documents below in order. Each document builds on the previous one and gradually moves from understanding what WeatherMule does to understanding how it is implemented.

## [Technical Documentation](technical/)

1. [quick-start.md](technical/quick-start.md)  
   Begin here. Provides a high-level introduction to WeatherMule and the quickest path to understanding the project.

2. [architecture.md](technical/architecture.md)  
   Describes the major components of WeatherMule and the responsibilities assigned to each subsystem.

3. [flow-storage.md](technical/flow-storage.md)  
   Describes the flow of data through the WeatherMule storage subsystem, from request to response.

4. [flow-application.md](technical/flow-application.md)  
   Describes the flow of data through the WeatherMule application, from weather station to storage and delivery.

5. [flow-weather-request.md](technical/flow-weather-request.md)  
   Describes the flow of a weather station request through the WeatherMule application, from request to response.

6. [flow-authentication.md](technical/flow-authentication.md)  
   Describes the flow of an authentication request through the WeatherMule application, from request to response.

7. [flow-upload-observation.md](technical/flow-upload-observation.md)  
   Describes the flow of an observation upload through the WeatherMule application, from request to response.

8. [flow-hole-processing.md](technical/flow-hole-processing.md)  
   Describes the flow of a hole processing request through the WeatherMule application, from request to response.

9. [source-map.md](technical/source-map.md)  
   Maps source files to responsibilities. Use this document when locating where specific functionality is implemented.

## [Project Story](project-story/)

The project stories describe how WeatherMule evolved, why design decisions were made, and lessons learned during development.

1. [silver-city-use-case.md](project-story/silver-city-use-case.md)  
   Describes the initial use case that motivated the WeatherMule project.

2. [life-of-a-weather-observation.md](project-story/life-of-a-weather-observation.md)  
   Explains how a weather observation moves through the system, from creation to storage and delivery.

3. [weather-station-data.md](project-story/weather-station-data.md)  
   Explains how the Ambient Weather protocol was discovered and validated.  

4. [pack-station-weather.md](project-story/pack-station-weather.md)  
   Describes how the PackStation weather station was integrated into WeatherMule.

5. [packbox-storage.md](project-story/packbox-storage.md)  
   Describes the design and implementation of the PackBox storage system.

6. [authentication.md](project-story/authentication.md)  
   Explains how authentication was implemented in WeatherMule.

7. [unload-weather.md](project-story/unload-weather.md)  
   Explains how the mule unloads weather observations to the pack station.

8. [holes.md](project-story/holes.md)  
   Explains how holes in the weather observation data are detected and processed.

## Hardware

Hardware-specific documentation is located in the hardware/ folder.

1. [ambient-weather-2000.md](hardware/ambient-weather-2000.md)  
2. [arduino.md](hardware/arduino.md)
3. [enclosure.md](hardware/enclosure.md)

## Reference Material

Reference documents contain vendor-specific information and supporting research used throughout the project.

1. [ambient-device-data-specs.md](reference/ambient-device-data-specs.md)  
   Reference documentation from Ambient Weather describing the weather station observation fields.

2. [protocol-discovery.md](reference/protocol-discovery.md)  
   Documents how the Ambient Weather protocol was analyzed, validated, and incorporated into WeatherMule.

---

## Related Project

### PackStationWeather

The destination at the end of the trail.

📖 [Building Our Own Pack Station](project-story/pack-station-weather.md)