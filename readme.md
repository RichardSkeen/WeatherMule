![Weather Mule](docs/images/weather-mule.png)

*Not interested in deep dive?*
```cpp
if (!Interested)
{
    Read("docs/readme.md");
}
else
{
    enjoy = true;
}
```



# WeatherMule

> **Store now. Deliver later.**

WeatherMule is a store-and-forward weather relay built for remote weather stations where internet connectivity is less reliable than the weather itself.

Designed for an off-grid cabin in Silver City, California. WeatherMule captures weather observations, stores them locally on an SD card, and delivers them to PackStationWeather when network connectivity becomes available.

Because storms don't wait for Wi-Fi.

## Features

- 🐴 Store-and-forward weather delivery
- 💾 Local SD-card archive
- 🌲 Designed for remote and off-grid deployments
- 🔋 Low-power Arduino Nano 33 IoT platform
- 🌨️ Built for harsh mountain weather
- 📦 Automatic recovery of missing observations
- 🚫 No cloud dependency required

## Why?

Most weather systems assume the network is always available.

WeatherMule assumes the weather is always available.

The difference matters when the interesting weather arrives at the same time the internet disappears.

## Documentation

- docs/project-story.md
- docs/design.md
- docs/silver-city-use-case.md
- docs/deployment.md

## The Guiding Principle

> Save the weather first.  
> Figure everything else out later.

---

**WeatherMule:** Off-grid. Low-power. Store-and-forward. Snow-aware. Mule-approved.