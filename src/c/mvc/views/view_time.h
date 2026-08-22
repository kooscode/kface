#pragma once

#include <pebble.h>

// Owns the time + date TextLayers. Pure rendering - no service subscriptions.
void view_time_create(Layer *parent, GRect bounds);
void view_time_destroy(void);
void view_time_set_time(const char *time_str);
void view_time_set_date(const char *date_str);
