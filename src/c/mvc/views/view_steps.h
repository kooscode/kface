#pragma once

#include <pebble.h>

// Owns the steps TextLayer. Pure rendering - no service subscriptions.
void view_steps_create(Layer *parent, GRect bounds);
void view_steps_destroy(void);
void view_steps_set_steps(uint32_t steps);
