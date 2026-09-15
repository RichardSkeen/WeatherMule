# Muck Stables

![Mucking Stables](images/mucking-stalls.png)

## Purpose

Muck Stables contains the WeatherMule hole-filling workflow.

The objective is simple:

- Keep hole processing lightweight.
- Avoid cadence calculations.
- Avoid date arithmetic.
- Avoid gap analysis on the mule.
- Let PackStationWeather determine what is missing.
- Let WeatherMule deliver whatever weather it possesses.

The mule should only need to:

1. Find the oldest hole.
2. Find the next observation after the hole bookmark.
3. Upload the observation.
4. Advance the bookmark.
5. Determine whether a hole contains observed weather or requires NotObserved.

---

# Hole Model

## Hole.h

Add:

```cpp
bool PartiallyFilled;
```

Meaning:

```text
false
    No observation has been uploaded
    between FromUtc and ToUtc.

true
    At least one observation has been
    uploaded between FromUtc and ToUtc.
```

This flag does not indicate that a hole has been completely filled.

The flag is only used to determine:

- I never had weather for this hole.
- I had at least some weather for this hole.

---

# Hole Processing

## ProcessHoles()

ProcessHoles() should:

1. Find the oldest hole.
2. Open the packbox containing FromUtc.
3. Locate the observation matching FromUtc.
4. Read the next observation.
5. Determine whether the next observation is ToUtc.
6. Upload observed weather or create NotObserved.
7. Update or remove the hole.

Pseudo-workflow:

```text
Oldest Hole
      ↓
Find FromUtc
      ↓
Read Next Observation
      ↓
Next Observation == ToUtc ?
```

If:

```text
PartiallyFilled == false
```

then:

```text
No observed weather exists
between FromUtc and ToUtc.
```

Send:

```text
NotObserved = 1
```

Remove hole.

If:

```text
PartiallyFilled == true
```

then:

```text
At least one observed weather record
was previously uploaded.
```

Remove hole.

PackStationWeather will recalculate any remaining holes during the next upload.

---

# StorageRoutines

## OpenPackBoxRead()

Responsibility:

```text
Open packbox containing FromUtc.
Locate row matching FromUtc.
Return the next observation row.
```

Hole processing should not need to know about files.

---

## ReadUntil()

Responsibility:

```text
Read until dateUtc matches FromUtc.
Return next observation row.
```

---

## TomorrowBoxFileName()

Responsibility:

```text
Given:

2026-09-12.box

Return:

2026-09-13.box
```

Used for end-of-packbox transitions.

---

# Packbox Boundary Handling

Example:

```text
2026-09-12.box

23:45
23:50
23:55 <- FromUtc
```

No additional rows exist.

ReadUntil() should:

```text
Open tomorrow's packbox
Read first row
Return first row
```

Example:

```text
2026-09-13.box

00:00
00:05
00:10
```

Return:

```text
00:00
```

Hole processing should not need to know whether the observation came from:

- current packbox
- tomorrow's packbox

StorageRoutines should handle that detail.

---

# Important Design Rule

WeatherMule does not know:

- reporting interval
- observation cadence
- expected observation count
- hole duration
- how many observations should exist

WeatherMule only knows:

```text
Find bookmark.
Read next observation.
Deliver cargo.
```

PackStationWeather remains the authority for:

- hole detection
- hole recalculation
- missing weather determination

The mule hauls cargo.

The station boss finds holes.