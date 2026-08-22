#pragma once

#include <pebble.h>

// Owns the temperature + condition TextLayers in the blank space between
// the two divider lines. Pure rendering - no service subscriptions, no
// knowledge of AppMessage/model types (see controller.c's inbox handler and
// model_weather_get()).
void view_weather_create(Layer *parent, GRect bounds);
void view_weather_destroy(void);
// valid=false shows placeholders ("--°"/"--") when no weather has arrived yet.
void view_weather_set_temp(bool valid, int16_t temp_c_tenths);
void view_weather_set_condition(bool valid, uint8_t condition_code);
// NULL/empty shows a "--" placeholder when no city has arrived yet.
void view_weather_set_location(const char *city);
