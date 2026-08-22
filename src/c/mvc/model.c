#include "model.h"

static const char *format_now(const char *format, char *buf, size_t buf_len)
{
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  strftime(buf, buf_len, format, tick_time);
  return buf;
}

const char *model_time_get(const char *format)
{
  static char s_buf[16];
  return format_now(format, s_buf, sizeof(s_buf));
}

const char *model_date_get(const char *format)
{
  static char s_buf[16];
  return format_now(format, s_buf, sizeof(s_buf));
}

uint32_t model_bpm_get(void)
{
  // Returns 0 if no reading is available yet.
  return (uint32_t)health_service_peek_current_value(HealthMetricHeartRateBPM);
}

uint8_t model_battery_get(void)
{
  return battery_state_service_peek().charge_percent;
}

uint32_t model_steps_get(enum steps_interval interval)
{
  if (interval == STEPS_INTERVAL_TODAY) {
    return (uint32_t)health_service_sum_today(HealthMetricStepCount);
  }

  // The SDK only ships health_service_sum_today() as a named convenience -
  // there's no _this_week/_this_month/_this_year equivalent. Build the same
  // thing ourselves with the generic health_service_sum(metric, start, end)
  // by computing midnight at the start of the requested period.

  time_t now = time(NULL);
  struct tm start_tm = *localtime(&now);
  start_tm.tm_hour = 0;
  start_tm.tm_min = 0;
  start_tm.tm_sec = 0;

  switch (interval) {
    case STEPS_INTERVAL_WEEK:
      // tm_wday: 0 = Sunday .. 6 = Saturday. Roll back to this week's Sunday.
      start_tm.tm_mday -= start_tm.tm_wday;
      break;
    case STEPS_INTERVAL_MONTH:
      start_tm.tm_mday = 1;
      break;
    case STEPS_INTERVAL_YEAR:
      start_tm.tm_mon = 0;
      start_tm.tm_mday = 1;
      break;
    default:
      break;
  }

  time_t start = mktime(&start_tm);
  return (uint32_t)health_service_sum(HealthMetricStepCount, start, now);
}

static WeatherData s_weather;

void model_weather_set(int16_t temp_c_tenths, uint8_t condition_code, bool is_day)
{
  s_weather.valid = true;
  s_weather.temp_c_tenths = temp_c_tenths;
  s_weather.condition_code = condition_code;
  s_weather.is_day = is_day;
}

WeatherData model_weather_get(void)
{
  return s_weather;
}

static char s_location[24] = "";

void model_location_set(const char *city)
{
  if (!city) {
    return;
  }
  snprintf(s_location, sizeof(s_location), "%s", city);
}

const char *model_location_get(void)
{
  return s_location;
}
