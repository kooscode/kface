#include "controller.h"
#include "model.h"
#include "views/view_border.h"
#include "views/view_time.h"
#include "views/view_battery.h"
#include "views/view_dividers.h"
#include "views/view_weather.h"
#include "views/view_heart.h"
#include "views/view_steps.h"
#include "views/view_bluetooth.h"

// Pull the latest value from every model and push it into every view.
// Deliberately not fine-grained: whichever event fires, everything refreshes.
static void refresh_ui(void)
{
  // %I is always zero-padded and Pebble's newlib strftime doesn't support
  // the GNU "%-I" no-pad extension, so strip a leading zero ourselves for
  // 12h mode. Also append %p (AM/PM) - only meaningful outside 24h mode.
  const char *time_str = model_time_get(clock_is_24h_style() ? "%H:%M" : "%I:%M%p");
  if (!clock_is_24h_style() && time_str[0] == '0') 
    time_str++;

  view_time_set_time(time_str);
  view_time_set_date(model_date_get("%b %d"));
  view_battery_set_percent(model_battery_get());
  view_bluetooth_set_connected(model_bluetooth_get());

  WeatherData weather = model_weather_get();
  view_weather_set_temp(weather.valid, weather.temp_c_tenths);
  view_weather_set_condition(weather.valid, weather.condition_code);
  view_weather_set_location(model_location_get());

 view_heart_set_bpm(model_bpm_get());
 view_steps_set_steps(model_steps_get(STEPS_INTERVAL_TODAY));

 //TEST DATA
//  view_heart_set_bpm(104);
//  view_steps_set_steps(4600);

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  refresh_ui();
}

static void health_handler(HealthEventType event, void *context)
{
  refresh_ui();
}

static void battery_handler(BatteryChargeState charge)
{
  refresh_ui();
}

static void bluetooth_handler(bool connected)
{
  refresh_ui();
}

// PebbleKit JS (src/pkjs/index.js) does the actual geolocation + Open-Meteo
// fetch and pushes the result in as an AppMessage - there's no public API
// for a watchapp to fetch weather itself. This just unpacks it into the
// model.
static void inbox_received_handler(DictionaryIterator *iterator, void *context)
{
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *code_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_CODE);
  Tuple *day_tuple = dict_find(iterator, MESSAGE_KEY_IS_DAY);
  Tuple *city_tuple = dict_find(iterator, MESSAGE_KEY_CITY);

  bool changed = false;

  if (temp_tuple && code_tuple && day_tuple) {
    model_weather_set((int16_t)temp_tuple->value->int32,
                       (uint8_t)code_tuple->value->uint32,
                       day_tuple->value->uint32 != 0);
    changed = true;
  }

  // Arrives in its own message (see fetchCity() in src/pkjs/index.js),
  // independent of the weather fields above.
  if (city_tuple) {
    model_location_set(city_tuple->value->cstring);
    changed = true;
  }

  if (changed) {
    refresh_ui();
  }
}

// The one place every system event subscription happens.
static void subscribe_events(void)
{
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  health_service_events_subscribe(health_handler, NULL);
  battery_state_service_subscribe(battery_handler);
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_handler
  });

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void unsubscribe_events(void)
{
  tick_timer_service_unsubscribe();
  health_service_events_unsubscribe();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  app_message_deregister_callbacks();
}

void controller_start(Layer *window_layer, GRect bounds)
{
  // view_border_create(window_layer, bounds);
  view_time_create(window_layer, bounds);
  view_battery_create(window_layer, bounds);
  view_dividers_create(window_layer, bounds);
  view_weather_create(window_layer, bounds);
  view_heart_create(window_layer, bounds);
  view_steps_create(window_layer, bounds);
  view_bluetooth_create(window_layer, bounds);

  subscribe_events();
  refresh_ui();
}

void controller_stop(void)
{
  unsubscribe_events();

  // view_border_destroy();
  view_time_destroy();
  view_battery_destroy();
  view_dividers_destroy();
  view_weather_destroy();
  view_heart_destroy();
  view_steps_destroy();
  view_bluetooth_destroy();
}
