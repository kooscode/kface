#pragma once

#include <pebble.h>

// Owns the heart-rate TextLayer. Pure rendering - no service subscriptions.
void view_heart_create(Layer *parent, GRect bounds);
void view_heart_destroy(void);
// Pass 0 to show a placeholder ("--") when no reading is available yet.
void view_heart_set_bpm(uint32_t bpm);
