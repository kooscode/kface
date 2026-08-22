//https://developer.repebble.com/docs/

#include <pebble.h>
#include "mvc/controller.h"

static Window *s_main_window;

static void main_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  controller_start(window_layer, layer_get_bounds(window_layer));
}

static void main_window_unload(Window *window)
{
  controller_stop();
}

static void init() {

  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set the background color
  window_set_background_color(s_main_window, GColorBlack);

  //setup HR read at 10min intervals
  health_service_set_heart_rate_sample_period(10 * 60);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers)
  {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

}

static void deinit()
{
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}
