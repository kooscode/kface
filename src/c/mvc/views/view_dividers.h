#pragma once

#include <pebble.h>

// Draws the two static horizontal divider lines: light blue below the date,
// blue above the HR/steps row. Static chrome - no data, no setter.
void view_dividers_create(Layer *parent, GRect bounds);
void view_dividers_destroy(void);
