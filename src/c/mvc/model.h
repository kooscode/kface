#pragma once

#include <pebble.h>

// Time/date: each returns a pointer to an internal static buffer holding the
// current time/date formatted with the given strftime format string. Like
// ctime(), the returned pointer is only valid until the next call.
const char *model_time_get(const char *format);
const char *model_date_get(const char *format);

// Returns the current heart rate in BPM, or 0 if no reading is available yet.
uint32_t model_bpm_get(void);

// Returns the current battery charge, 0-100.
uint8_t model_battery_get(void);

// Returns true if the watch currently has a Bluetooth connection to the phone.
bool model_bluetooth_get(void);

// Returns the step count accumulated since midnight.
enum steps_interval {
  STEPS_INTERVAL_TODAY,
  STEPS_INTERVAL_WEEK,
  STEPS_INTERVAL_MONTH,
  STEPS_INTERVAL_YEAR
};
uint32_t model_steps_get(enum steps_interval interval);

// Weather. There's no public API for a watchapp to fetch weather itself -
// PebbleKit JS (src/pkjs/index.js) does the geolocation + Open-Meteo HTTP
// fetch and pushes the result in over AppMessage; model_weather_set() is
// controller.c's inbox handler storing what arrives. model_weather_get()
// returns valid=false (with the rest of the struct zeroed) until the first
// message arrives.
typedef struct {
  bool valid;
  int16_t temp_c_tenths;
  uint8_t condition_code;  // Open-Meteo/WMO weather code
  bool is_day;
} WeatherData;

void model_weather_set(int16_t temp_c_tenths, uint8_t condition_code, bool is_day);
WeatherData model_weather_get(void);

// Reverse-geocoded city name for the weather location - also pushed from
// PebbleKit JS over AppMessage (Nominatim, see src/pkjs/index.js), and
// independent of the weather fields above (it can arrive in its own
// message). Returns "" until the first message arrives.
void model_location_set(const char *city);
const char *model_location_get(void);
