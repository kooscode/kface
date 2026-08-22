#pragma once

#include <pebble.h>

// Draws a blue border ring around the whole window. Static chrome - no data,
// no setter.
void view_border_create(Layer *parent, GRect bounds);
void view_border_destroy(void);
