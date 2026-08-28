# kface — Pebble watchface

A Pebble Time 2 (emery) watchface written in C against the Pebble SDK,
built with a small MVC structure. Currently displays time, date, battery,
Bluetooth connection state, weather, heart rate, and steps-since-midnight.

## SDK docs

Reference docs: https://developer.repebble.com/docs/c (reachable via
WebFetch). Five top-level modules: Foundation, Graphics, User Interface,
Worker, Standard C. General guide/tutorials: https://developer.repebble.com

When docs summaries seem uncertain or contradict observed behavior, cross-check
against the actual installed SDK header instead of trusting the doc fetch:
`~/.local/share/pebble-sdk/SDKs/current/sdk-core/pebble/emery/include/pebble.h`
(also `pebble_fonts.h` in the same dir for exact `FONT_KEY_*` names, and
`build/emery/pebble_app.ld.auto` for the actual linker script). This caught
several cases where the doc-fetch summary was incomplete or slightly wrong.

## Project layout

```
src/c/
  kface.c                  App/window lifecycle only: init(), main(), window
                            load/unload handlers that call controller_start()/
                            controller_stop(). No model/view/event knowledge.
  mvc/
    controller.{c,h}       The ONLY place system event subscriptions happen
                            (tick_timer_service_subscribe, health_service_
                            events_subscribe, battery_state_service_subscribe,
                            connection_service_subscribe,
                            app_message_register_inbox_received/_open).
                            inbox_received_handler() unpacks the AppMessage
                            PebbleKit JS sends (see pkjs/index.js below) into
                            model_weather_set(). refresh_ui() is called by any
                            event, unconditionally re-pulls every model value
                            and pushes it into every view - deliberately not
                            fine-grained/per-event-type.
    model.{c,h}             All data access, one file: model_time_get(format),
                            model_date_get(format) (each returns const char*
                            into an internal static buffer, ctime()-style),
                            model_bpm_get(), model_steps_today_get(),
                            model_battery_get(), model_bluetooth_get()
                            (connection_service_peek_pebble_app_connection()).
                            model_weather_get()/
                            model_weather_set() are push-based (see
                            controller.c's inbox_received_handler below)
                            rather than pulled from a Pebble service. No
                            layers, no subscriptions.
    views/
      view_border.{c,h}     draws the light blue border ring (static chrome,
                             no data/setter)
      view_time.{c,h}       owns time+date TextLayers
      view_battery.{c,h}    owns the battery meter (yellow 1px border, green
                             fill for charge left)
      view_dividers.{c,h}   draws the two light blue divider lines around
                             the weather area (static chrome, no data/setter)
      view_weather.{c,h}    owns temperature+condition TextLayers in the
                             blank space between the divider lines
      view_heart.{c,h}      owns BPM TextLayer (yellow), bottom row left half
      view_steps.{c,h}      owns steps TextLayer (green), bottom row right half
      view_bluetooth.{c,h}  small filled-circle indicator, top-right corner
                             inside the border - green when connected, red
                             when not
      Each view: _create(Layer *parent, GRect bounds) / _destroy() / _set_*().
      Each owns its own static display buffer internally (TextLayer stores
      the pointer it's given, not a copy - views must not point it at a
      caller's transient stack buffer). Row/line Y-coordinates in these
      files are frequently tuned against actual screenshots, not just box
      math - see the comments in view_time.c/view_dividers.c for why (system
      font glyphs have significant, font-specific top/bottom padding baked
      into their box that box arithmetic alone doesn't account for).
  pkjs/
    index.js                PebbleKit JS companion, runs on the phone. Does
                             geolocation (navigator.geolocation) + an Open-
                             Meteo HTTP fetch, then Pebble.sendAppMessage()s
                             the result to the watch using the message_keys
                             module (require('message_keys')) rather than
                             hardcoded numeric keys. This is the only way to
                             get weather onto the watch - see the "no public
                             API" gotcha below. Modeled on jhatax/at-a-glance's
                             src/pkjs/weather_location.js, trimmed to just
                             current temp/condition/day-night (no location
                             name/reverse geocoding).
resources/        Images, fonts, other bundled resources (empty so far)
package.json      App metadata: UUID, sdkVersion, targetPlatforms, resources,
                  messageKeys (TEMPERATURE/WEATHER_CODE/IS_DAY, consumed on
                  the C side as MESSAGE_KEY_* from the generated
                  build/include/message_keys.auto.h - run a clean build
                  after changing this list, an incremental build won't
                  regenerate that header)
wscript           waf build rules - globs src/c/**/*.c recursively (new
                  subfolders under src/c/ need no build config changes) and
                  src/pkjs/**/*.js as the JS bundle, entry src/pkjs/index.js
build/            Generated output - do not hand-edit
```

`package.json`'s `pebble` block is the source of truth for app config: watchface
(`watchapp.watchface: true`), UUID `c3ba829f-0d5f-4b94-8848-16822cbec076`,
`targetPlatforms: ["emery"]` (Pebble Time 2 only). `build/appinfo.json` is
generated from it — don't edit directly.

## Build & run

The `pebble` CLI needs its venv active first:
```sh
source /data/virtual/python/pebble/bin/activate
```

```sh
pebble build                          # build for all targetPlatforms
pebble install --emulator emery       # install on the emery emulator
pebble install --phone <ip>           # install to the paired phone over wifi
pebble logs --phone <ip> -v           # live app logs (also works with --emulator emery)
pebble screenshot --emulator emery out.png   # grab a screenshot from the emulator
```

If a shell session is already running inside the pebble venv (check the
prompt / `which pebble`), don't re-`source activate` — just run `pebble
build && pebble install --phone <ip>` directly.

The user's phone is normally reachable at `192.168.1.178` on the local wifi
(ping it to confirm before assuming it's down). `pebble install --phone <ip>`
reporting "App install succeeded" only means the .pbw was transferred — it
does **not** mean the watchface became visible/active on the phone. If
nothing visibly changes after a successful install, that's not necessarily a
build/deploy problem: check on the phone itself that Developer Connection is
still toggled on in the Pebble app and that this watchface has been
selected/set active (a fresh install doesn't auto-activate it).
The emulator is normally already running in the background (`qemu-pebble` +
`pypkjs` processes) — check with `ps aux | grep pebble` before trying to
launch a new one.

The user's phone IP occasionally becomes unreachable (dev connection toggled
off, or IP changed) — `pebble install --phone <ip>` fails with
`[Errno 111] Connection refused` in that case; ask the user to check the
Pebble app's Developer Connection setting rather than assuming a code problem.

## Hardware gotchas learned the hard way (real emery device, not just emulator)

- Watch faces does NOT support touch gestures - only apps do.
- Only DIY Weather available - Implemented such in `src/pkjs/index.js` (geolocation +
  Open-Meteo) / `view_weather.c` + `model_weather_set()`/`model_weather_get()`
  on the C side. Confirmed working end-to-end against the emery emulator.
- Geolocation - `pypkjs` does answer real `navigator.geolocation` + `XMLHttpRequest`
  calls, it's not just stubbed and Open-Meteo used to for resolving city

## Conventions established in this codebase

- MVC split (see Project layout above) — new features should add a model
  getter, a view module, and wire it into `controller.c`'s `refresh_ui()` and
  `subscribe_events()`, rather than growing `kface.c`.
- Views own their own `TextLayer`s and destroy them in `_destroy()`.
- Function naming is `module_object_action`, e.g. `model_time_get`,
  `view_time_create`, `view_heart_set_bpm`. Module first, then the thing
  being acted on, then the verb last.
- Colors/fonts in use: time = white `FONT_KEY_ROBOTO_BOLD_SUBSET_49`, date =
  white `FONT_KEY_BITHAM_42_BOLD`, weather temp = white
  `FONT_KEY_BITHAM_30_BLACK`, weather condition = light blue
  `FONT_KEY_GOTHIC_24_BOLD`, heart = yellow, steps = green, both
  `FONT_KEY_BITHAM_34_MEDIUM_NUMBERS`. Border/dividers/battery-meter-border =
  light blue (`GColorPictonBlue`), battery fill = green. Black background
  throughout.
- New bundled assets go under `resources/` and must be registered in
  `package.json`'s `pebble.resources.media`.
