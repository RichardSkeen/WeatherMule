# WeatherMule Documentation

This documentation is organized into technical references, project stories, hardware references, and supporting materials.

For readers new to the project, follow the documents below in order. Each document builds on the previous one and gradually moves from understanding what WeatherMule does to understanding how it is implemented.

## [Technical Documentation](technical/)

1. [quick-start.md](technical/quick-start.md)

   Begin here. Provides a high-level introduction to WeatherMule and the quickest path to understanding the project.

2. [work-flow.md](technical/work-flow.md)

   Explains how observations move through the system, from weather station reception to storage, delivery, authentication, recovery, and hole processing.

3. [architecture.md](technical/architecture.md)

   Describes the major components of WeatherMule and the responsibilities assigned to each subsystem.

4. [source-map.md](technical/source-map.md)

   Maps source files to responsibilities. Use this document when locating where specific functionality is implemented.

5. [protocol-discovery.md](technical/protocol-discovery.md)

   Documents how the Ambient Weather protocol was analyzed, validated, and incorporated into WeatherMule.

## Project Story

The project stories describe how WeatherMule evolved, why design decisions were made, and lessons learned during development.

1. [silver-city-use-case.md](project-story/silver-city-use-case.md)  
   Describes the initial use case that motivated the WeatherMule project.

2. [weather-station-data.md](project-story/weather-station-data.md)  
   Explains how the Ambient Weather protocol was discovered and validated.  

3. [pack-station-weather.md](project-story/pack-station-weather.md)  
   Describes how the PackStation weather station was integrated into WeatherMule.

4. [packbox-storage.md](project-story/packbox-storage.md)  
   Describes the design and implementation of the PackBox storage system.

- Project Evolution
- Engineering Incidents
- Development Culture

## Hardware

Hardware-specific documentation is located in the hardware/ folder.


1. [ambient-weather-2000.md](hardware/ambient-weather-2000.md)  
2. [arduino.md](hardware/arduino.md)
3. [enclosure.md](hardware/enclosure.md)

## Reference Material

Reference documents contain vendor-specific information and supporting research used throughout the project.

1. [ambient-device-data-specs.md](reference/ambient-device-data-specs.md)

   Reference documentation describing the weather station observation fields used throughout the project. This document is shared by both the technical documentation and project stories.
