
```mermaid
flowchart TD

    A[Weather Station] --> B[WeatherRequest]
    B --> C[Save Raw Request]
    C --> D[Packbox]

    style B fill:#90EE90
    style C fill:#90EE90
    style D fill:#90EE90

    D --> E{Design.md Path}

    E --> F[Update current.json]
    F --> G[Maintain Statistics]
    G --> H[Determine Missing Ambient Data]
    H --> I[Synchronize Ambient]

    style F fill:#FFD580
    style G fill:#FFD580
    style H fill:#FFB6B6
    style I fill:#FFB6B6

    E --> J{Current Code Path}

    J --> K[Upload Latest Observation]
    K --> L[CheckIn API]
    L --> M[PackStationWeather]
    M --> N[Receive Hole List]
    N --> O[Store Holes]

    style K fill:#90EE90
    style L fill:#90EE90
    style M fill:#90EE90
    style N fill:#90EE90
    style O fill:#90EE90

    O --> P[Process Holes]

    style P fill:#FFD580
