# WeatherMule Project Structure

WeatherMule documentation is organized into four main sections. Each section serves a different purpose and is intended for a different kind of reader.

## Documentation Map

```text
docs/
├── hardware/
├── project-story/
│   └── silver-city-use-case.md
├── reference/
│   └── ambient-device-data-specs.md
├── technical/
│   ├── architecture.md
│   ├── protocol-discovery.md
│   ├── quick-start.md
│   ├── source-map.md
│   └── work-flow.md
└── readme.md
```

## Technical Documentation

The ../technical/ section provides the direct, no-fluff explanation of WeatherMule.

These documents describe what the system does, how its components work together, and where responsibilities are implemented in the source code.

- ../technical/architecture.md  
  Describes the major components, design principles, data flow, authentication, packboxes, holes, and recovery behavior.

- ../technical/protocol-discovery.md  
  Documents the discovery and interpretation of the weather station's HTTP reporting protocol.

- ../technical/quick-start.md  
  Provides the shortest path from obtaining the project to receiving and storing weather observations.

- ../technical/source-map.md  
  Lists the source files and summarizes the responsibility of each file.

- ../technical/work-flow.md  
  Describes the operational flow of WeatherMule, including observation handling, storage, upload, and hole recovery.

Readers who need answers without the project history or mule lore should begin with the technical documentation.

## Hardware Documentation

The ../hardware/ section describes the physical components used to build WeatherMule.

These documents explain:

- What hardware was selected
- How the components are connected
- Why each component was chosen
- Which choices were practical rather than mandatory
- What may be changed in future versions

The hardware documentation distinguishes between requirements imposed by the software and decisions made for the original WeatherMule build.

## Reference Documentation

The ../reference/ section contains external specifications and supporting technical material used during development.

- ../reference/ambient-device-data-specs.md  
  Records Ambient Weather device fields and related protocol information used by WeatherMule.

Reference documents preserve information that may be useful when interpreting observations, extending protocol support, or troubleshooting communication with weather station hardware.

## Project Story

The ./ section explains why WeatherMule exists and how the operating environment shaped the project.

Unlike the technical documentation, these documents include the reasoning, discoveries, mistakes, terminology, and lore that developed alongside the software.

- [`silver-city-use-case.md`](silver-city-use-case.md)  
  Introduces the Silver City environment, the off-grid cabin, the community network, and the weather conditions that made reliable local observation storage necessary.

Additional project-story documents will expand on the development journey, important design decisions, notable incidents, and the origin of WeatherMule terminology.

## Documentation Paths

WeatherMule provides two primary ways through the documentation.

docs/readme.md is an index in reading order for all the documentation.

### Direct Technical Path

For readers who need to understand, build, operate, or troubleshoot WeatherMule:

1. docs/technical/quick-start.md
2. docs/technical/architecture.md
3. docs/technical/work-flow.md
4. docs/technical/source-map.md
5. docs/technical/protocol-discovery.md

### Project Story Path

For readers interested in the environment, reasoning, discoveries, and lore behind WeatherMule:

1. docs/project-story/silver-city-use-case.md

2. Future project-story chapters
3. Supporting images and incident illustrations

The technical path explains how WeatherMule works.

The project-story path explains why WeatherMule became what it is.