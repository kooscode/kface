#pragma once

#include <pebble.h>

// Creates all views under window_layer and subscribes to the system events
// that keep them updated. Call from the window's load handler.
void controller_start(Layer *window_layer, GRect bounds);

// Unsubscribes from system events and destroys all views. Call from the
// window's unload handler.
void controller_stop(void);
