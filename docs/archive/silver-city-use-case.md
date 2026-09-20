# Silver City Use Case

## Why WeatherMule Exists

WeatherMule was not created as a general-purpose weather platform.

It was created to solve a specific problem in a specific place.

That place is Silver City, California.

The weather station that inspired WeatherMule can be viewed here:

[Silver City Weather Station](https://ambientweather.net/share/4XV3OU)

Elevation: 6,900 feet.

Internet availability: Maybe.

Snow availability: Definitely.

## The Environment

The primary deployment target is an off-grid cabin operating from solar power and battery storage.

Internet connectivity is provided through a shared community Wi-Fi network. The network may become unavailable due to:

- Power limitations
- Equipment failures
- Weather conditions
- Seasonal access limitations
- Upstream internet outages

Unlike most connected devices, a weather station continues collecting observations whether the internet is available or not.

The weather does not stop because the internet is down.

## The Problem

Traditional cloud-connected weather stations assume that observations can be uploaded immediately.

When connectivity is lost, observations may never reach their destination.

This is particularly problematic because some of the most interesting weather events often occur during storms, power interruptions, and poor network conditions.

Examples include:

- Overnight low temperatures
- Winter storms
- High wind events
- Extended snowfall periods

These are often the exact observations that users want to preserve.

## WeatherMule's Job

WeatherMule acts like a Sierra Nevada pack mule.

When the road is open:

```text
Weather Station
        |
        v
Ambient Weather
```

When the road is closed:

```text
Weather Station
        |
        v
WeatherMule
        |
   Packboxes (.box)
```

When the road opens again:

```text
Packboxes (.box)
        |
        v
Ambient Weather
```

WeatherMule does not create weather.

WeatherMule does not modify weather.

WeatherMule carries weather observations until they can be delivered.

## Design Decisions Driven By This Use Case

### Arduino Hardware

The target environment values reliability, simplicity, low power consumption, and long unattended operation.

An Arduino Nano 33 IoT consumes far less power than traditional computers and can remain operational for long periods on limited power budgets.

### SD Card Storage

Observations must survive:

- Internet outages
- Ambient Weather outages
- Router restarts
- Power interruptions

WeatherMule stores observations locally before attempting delivery.

### Daily Packboxes

Observations are stored in daily packboxes using the `.box` file extension.

Examples:

```text
20260815.box
20260816.box
20260817.box
```

Each packbox contains the weather cargo collected during a single day.

### The Wrangler's Notebook

WeatherMule maintains a small state file called:

```text
current.json
```

The file contains:

- Current packbox
- Daily statistics
- Synchronization state
- Delivery status

This allows WeatherMule to recover quickly after power loss without scanning every packbox.

## Success Criteria

A successful WeatherMule deployment should be able to:

- Continue collecting observations during extended internet outages.
- Survive power interruptions without data loss.
- Operate unattended for months.
- Preserve significant weather events.
- Synchronize missing observations when connectivity returns.
- Remain simple enough to troubleshoot years later.

## Guiding Principle

When faced with a design decision:

**Save the weather first. Figure everything else out later.**

Because weather observations can be stored and delivered later.

Lost observations are gone forever.