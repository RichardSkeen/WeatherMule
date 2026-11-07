# Bug Report: Cross-Packbox Hole Boundary Returned as a Hole Fill

## Status

Fixed in WeatherMule version 0.1.3 and under field verification.

## Summary

WeatherMule could incorrectly return the first observation from the next Packbox as a hole-fill observation without first verifying that the observation was strictly inside the server-provided hole.

The defect occurred when hole processing reached the end of one Packbox and `ReadUntil()` returned `NEXT_FILE`. The next-Packbox path opened the next available `.box` file and returned its first observation directly.

If that first observation was exactly equal to `ToDateRaw`, WeatherMule attempted to send the hole's existing far endpoint to `FillHoleApi()` as though it were a missing observation.

The correct result in that case is `NOT_FOUND`, because no observation exists strictly between `FromDateRaw` and `ToDateRaw`.

## Architecture Rule

PackStationWeather owns hole discovery.

A hole returned to WeatherMule has two known observations:

```text
FromDateRaw < missing observations, if any < ToDateRaw
```

Both endpoints already exist on PackStationWeather. WeatherMule must never attempt to fill a hole with either endpoint.

WeatherMule's responsibility is only to search Packboxes for an observation whose Ambient `dateutc` is strictly between the two raw endpoint values.

If no such observation exists anywhere between the endpoints, WeatherMule reports the hole as NotObserved.

Packbox boundaries have no synchronization meaning. A hole may span one Packbox, many Packboxes, missing calendar days, or long periods without observations.

## Observed Failure

During a large recovery test, PackStationWeather returned this hole:

```text
FromDateRaw = 2026-09-30+13:11:45
ToDateRaw   = 2026-10-01+02:59:38
```

The physical Packbox data showed:

```text
2026-09-30.box
...
2026-09-30+13:11:45    <-- FromDateRaw
EOF

              no observations

2026-10-01.box
2026-10-01+02:59:38    <-- ToDateRaw
2026-10-01+03:04:39
...
```

Serial logging showed:

```text
-opened packbox: 2026-09-30.box
    -Hole: ... fromRaw: 2026-09-30+13:11:45    toRaw: 2026-10-01+02:59:38
    -Line's date matches FromDateRaw: 2026-09-30+13:11:45
-ReadUntil result: NEXT_FILE
```

WeatherMule then read the first observation from `2026-10-01.box`:

```text
dateutc=2026-10-01+02:59:38
```

That timestamp was exactly equal to `ToDateRaw`, but the next-Packbox path returned it as a hole-fill observation.

WeatherMule subsequently attempted `FillHoleApi()` and PackStationWeather returned:

```text
HTTP/1.1 500 Internal Server Error
```

## Expected Behavior

For the hole:

```text
2026-09-30+13:11:45 < observation < 2026-10-01+02:59:38
```

there are no Packbox observations between the endpoints.

WeatherMule should therefore return:

```text
NOT_FOUND
```

and report the hole through `NotObservedApi()`.

The first record of the next Packbox must be subjected to the same `ToDateRaw` boundary check used while scanning within a Packbox.

## Root Cause

The in-file `ReadUntil()` logic correctly rejected observations at or beyond the far endpoint:

```cpp
if (useDateRaw >= fillHole.ToDateRaw)
{
    return ReadUntil_NotFound;
}
```

However, when `ReadUntil()` reached EOF and returned `ReadUntil_NextFile`, `OpenPackboxRead()` opened the next Packbox and returned its first line without performing the equivalent `ToDateRaw` comparison.

The two paths therefore implemented different hole-boundary rules.

## Fix

The `NEXT_FILE` handling in `StorageRoutines.h` was changed so the first observation read from the next Packbox is parsed and its raw Ambient `dateutc` is compared with `fillHole.ToDateRaw` before it can be returned as cargo.

The required boundary rule is:

```cpp
if (useDateRaw >= fillHole.ToDateRaw)
{
    return ReadUntil_NotFound;
}
```

Only an observation satisfying this condition may be returned for hole filling:

```text
FromDateRaw < useDateRaw < ToDateRaw
```

## Why Raw Dates Matter

Hole traversal uses `FromDateRaw` and `ToDateRaw` because those values correspond directly to Ambient Weather's original `dateutc` values stored in the Packbox records.

For example:

```text
dateutc=2026-09-30+13:11:45
```

can be compared directly with the hole's raw endpoint without introducing UTC parsing, formatting, or normalization differences.

## Discovery

The bug was found while rebuilding PackStationWeather from a nearly empty database using WeatherMule's historical Packboxes.

The test database was reduced to one genuine historical observation:

```text
2026-09-28+23:09:09
```

A new live observation was then unloaded by WeatherMule, causing PackStationWeather to return a very large hole spanning from September 28 to November 6.

WeatherMule successfully replayed historical observations at high speed using its Packbox seek/byte-offset logic. Live Ambient observations periodically interrupted recovery as designed. Each live observation caused WeatherMule to discard its current holes, unload the live observation, and receive a fresh authoritative hole list from PackStationWeather.

This large-scale recovery exercise eventually produced a legitimate hole spanning two Packboxes with no observations between its endpoints, exposing the `NEXT_FILE` boundary defect.

## Related Test Incident: The Wrangler Timestamp

An earlier recovery attempt appeared to fail at:

```text
2026-09-28+23:44:16
```

Investigation found an observation in the PackStationWeather database with:

```text
2026-09-28+23:48:00
```

That observation did not exist in the WeatherMule Packbox. The real Packbox sequence was:

```text
2026-09-28+23:39:15
2026-09-28+23:44:16
2026-09-28+23:49:17
2026-09-28+23:54:18
2026-09-28+23:59:19
```

The synthetic `23:48:00` database record split the server's hole list at an endpoint WeatherMule could never find in its Packboxes. Removing the bad test record allowed recovery to proceed through `23:49:17` normally.

This was test-data contamination rather than the cross-Packbox software defect documented above, but it helped expose an important synchronization invariant:

> Server-provided hole endpoints should correspond to genuine stored observations.

In project lore, this incident is formally attributed to the Wrangler, who apparently believed `23:48:00` "sounded right."

## Verification

WeatherMule version 0.1.3 was deployed with the `NEXT_FILE` boundary fix.

Verification procedure:

1. Reduce the PackStationWeather test database to the known genuine seed observation.
2. Allow a new Ambient observation to arrive normally.
3. Allow WeatherMule to unload the live observation and receive the resulting hole list.
4. Let WeatherMule replay historical Packbox observations normally.
5. Confirm ordinary same-Packbox hole fills continue to succeed.
6. Confirm recovery continues across Packbox boundaries.
7. Confirm a cross-Packbox hole whose next available observation equals `ToDateRaw` produces `NOT_FOUND` rather than `FillHoleApi()`.
8. Confirm WeatherMule reports that hole through `NotObservedApi()`.
9. Confirm no HTTP 500 is generated for an attempt to fill a hole using its existing far endpoint.
10. Confirm live observations can interrupt recovery, replace the current hole list through normal unload, and recovery continues from the fresh instructions.

## Regression Cases

The following cases should remain covered during future recovery testing:

### Observation exists in the same Packbox

```text
FROM -> observation -> TO
```

Expected: upload the observation with `FillHoleApi()`.

### No observation exists in the same Packbox

```text
FROM -> TO
```

Expected: `NOT_FOUND`, then `NotObservedApi()`.

### Observation exists in a later Packbox

```text
FROM -> EOF

next/later Packbox -> observation -> TO
```

Expected: continue across Packboxes and upload the observation.

### Next Packbox begins with ToDateRaw

```text
FROM -> EOF

next Packbox -> TO
```

Expected: `NOT_FOUND`. `TO` must never be sent to `FillHoleApi()`.

### Calendar days are missing

```text
FROM -> EOF

(no Packbox for several days)

later Packbox -> observation -> TO
```

Expected: calendar gaps do not imply NotObserved. Continue searching for stored observations until the hole boundary is reached.

### Live weather interrupts historical recovery

Expected:

1. Preserve the live observation first.
2. Forget the existing hole assignments.
3. Unload the live observation.
4. Accept the fresh hole list from PackStationWeather.
5. Resume recovery using the new list.

## Files Involved

Primary fix:

```text
StorageRoutines.h
```

Related hole orchestration:

```text
Hole.h
HoleRoutines.h
```

The defect itself was in Packbox traversal, not in the higher-level hole orchestration.

## Lesson Learned

A Packbox transition must not change the definition of a hole.

Whether observations are in one file, the next day's file, or a Packbox several days later, WeatherMule follows one rule:

```text
FromDateRaw < candidate dateutc < ToDateRaw
```

If no candidate satisfies that rule, the hole is NotObserved.

And, for the historical record:

> Never trust a Wrangler with a timestamp.
