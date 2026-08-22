#pragma once

#include <pebble.h>

// Owns the thin battery-meter rect: 1px yellow border, green fill for
// charge remaining, blank (background) for charge used. Pure rendering -
// no service subscriptions.
void view_battery_create(Layer *parent, GRect bounds);
void view_battery_destroy(void);
// percent is 0-100.
void view_battery_set_percent(uint8_t percent);
