
## Example PackBox Rows

A PackBox stores the original HTTP request lines received from a weather station. Each line is preserved exactly as received so that observations can be processed, replayed, or audited later.

Example PackBox contents:

```text
GET /data/report/&PASSKEY=0000000Phony000Key00000000000&stationtype=AMBWeatherPro_V5.1.5&dateutc=2026-09-15+19:56:08:10&tempf=77.9&humidity=43&windspeedmph=7.38&windgustmph=12.53&maxdailygust=15.88&winddir=330&winddir_avg10m=330&uv=7&solarradiation=820.62&hourlyrainin=0.000&eventrainin=0.000&dailyrainin=0.000&weeklyrainin=0.000&monthlyrainin=0.000&yearlyrainin=18.634&battout=1&tempinf=74.1&humidityin=48&baromrelin=28.886&baromabsin=28.886&battin=1 HTTP/1.1

GET /data/report/&PASSKEY=0000000Phony000Key00000000000&stationtype=AMBWeatherPro_V5.1.5&dateutc=2026-09-15+20:01:08:11&tempf=77.7&humidity=44&windspeedmph=3.80&windgustmph=13.65&maxdailygust=15.88&winddir=25&winddir_avg10m=301&uv=8&solarradiation=826.66&hourlyrainin=0.000&eventrainin=0.000&dailyrainin=0.000&weeklyrainin=0.000&monthlyrainin=0.000&yearlyrainin=18.634&battout=1&tempinf=74.3&humidityin=48&baromrelin=28.880&baromabsin=28.880&battin=1 HTTP/1.1

GET /data/report/&PASSKEY=0000000Phony000Key00000000000&stationtype=AMBWeatherPro_V5.1.5&dateutc=2026-09-15+20:06:08:12&tempf=78.1&humidity=42&windspeedmph=9.62&windgustmph=12.53&maxdailygust=15.88&winddir=332&winddir_avg10m=319&uv=8&solarradiation=836.13&hourlyrainin=0.000&eventrainin=0.000&dailyrainin=0.000&weeklyrainin=0.000&monthlyrainin=0.000&yearlyrainin=18.634&battout=1&tempinf=74.3&humidityin=48&baromrelin=28.877&baromabsin=28.877&battin=1 HTTP/1.1
```

### Notes

- Each row is a complete HTTP request captured from the weather station.
- Rows are stored in the order received.
- The original parameter names and values are preserved.
- Observations in this example occur at five-minute intervals.
- WeatherMule does not require knowledge of individual fields when storing a PackBox. The raw request is retained and can be parsed later.
- Different station firmware versions may send additional or different fields.
- The PASSKEY shown above is fictitious and intended only for documentation.

### Why PackBox Stores the Raw Request

PackBox is intended to preserve the original observation exactly as received. Storing the raw request provides:

- A complete audit trail.
- Protection against parser bugs.
- Support for future parser improvements.
- The ability to replay historical observations.
- Compatibility with new station firmware that introduces additional fields.

PackBox is an immutable record of what the weather station actually sent. Downstream processing extracts measurements and creates structured observations from the preserved request.

