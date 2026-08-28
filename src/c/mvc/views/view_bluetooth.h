#pragma once

#include <pebble.h>

void view_bluetooth_create(Layer *parent, GRect bounds);
void view_bluetooth_destroy(void);
void view_bluetooth_set_connected(bool connected);
