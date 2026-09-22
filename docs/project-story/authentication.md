## Check-In at the Station

When PackStationWeather was first conceived, authentication appeared simple.

WeatherMule already had a unique API key. The mule could attach that key to every request and PackStationWeather could decide whether the cargo should be accepted.

Problem solved.

At least until the question became:

What happens if that key is ever stolen?

A permanent credential is a little like giving somebody the only key to the supply shed. It works, but every trip requires carrying the same key, and if that key is lost, copied, or exposed, replacing it becomes unpleasant.

The solution was to separate identity from daily operations.

### Arriving at the Pack Station

*Implementation: [ApiClientRoutines.h](../../ApiClientRoutines.h)*

Before the mule can unload cargo, it stops at the pack station and checks in.

The mule presents its API key to:

```http
   GET /api/checkin
```

If PackStationWeather recognizes the mule, it issues two new credentials:

- A session token
- A refresh token

The mule then stores those credentials in memory and uses them for normal operations.

The permanent API key goes back into the saddlebag and remains unused unless it is needed again.

### Why Not Use the API Key For Everything?

The API key identifies the mule.

The session token authorizes the trip.

Those are different responsibilities.

Using temporary session credentials offers several advantages:

- The API key is transmitted less often.
- Sessions can expire automatically.
- Compromised sessions can be replaced.
- Authentication behavior can evolve without replacing every deployed mule.

Most importantly, the mule can lose its temporary paperwork without losing its identity.

### Mountain Weather and Memory Loss

WeatherMule operates in places where power failures are not theoretical.

A winter storm does not care whether a session token expires gracefully.

If the power disappears, the mule loses:

- Session token
- Refresh token

The mule retains:

- API key
- SD card
- Packboxes
- Historical weather

When power returns, the mule simply visits the check-in station again.

No recovery procedure is required.

No token files must be restored.

No credentials must be copied back from a backup.

The mule introduces itself, receives fresh travel papers, and resumes hauling weather.

### Refreshing Travel Papers

Eventually a session token expires.

When PackStationWeather refuses a request with:

    401 Unauthorized

the mule first attempts to renew its session using the refresh token.

If that succeeds, new credentials are issued and the failed operation is retried.

If the refresh token has also expired, the mule falls back to the original API key and performs a completely new check-in.

From the mule's perspective, this is simply another stop at the pack station.

### The Design Principle

The API key proves who the mule is.

The session token proves it recently checked in.

The refresh token allows travel papers to be renewed without repeatedly exposing the permanent credential.

The mule carries weather cargo.

The pack station keeps track of who is allowed through the gate.