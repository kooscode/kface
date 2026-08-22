#include "view_steps.h"

static TextLayer *s_steps_layer;

// Bottom row, right half - see view_heart.c for the matching left half and
// the shared ROW_Y/ROW_HEIGHT.
#define ROW_Y 186
#define ROW_HEIGHT 44
#define OFFSET_LEFT 20

void view_steps_create(Layer *parent, GRect bounds)
{

  s_steps_layer = text_layer_create(GRect((bounds.size.w / 2) - OFFSET_LEFT, ROW_Y, (bounds.size.w / 2) + OFFSET_LEFT, ROW_HEIGHT));
  text_layer_set_background_color(s_steps_layer, GColorClear);
  text_layer_set_text_color(s_steps_layer, GColorGreen);
  text_layer_set_font(s_steps_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
  text_layer_set_text_alignment(s_steps_layer, GTextAlignmentRight);
  layer_add_child(parent, text_layer_get_layer(s_steps_layer));

  view_steps_set_steps(0);
}

void view_steps_destroy(void)
{
  text_layer_destroy(s_steps_layer);
}

void view_steps_set_steps(uint32_t steps)
{
  static char s_buf[16];
  snprintf(s_buf, sizeof(s_buf), "🤘 %lu", steps);
  text_layer_set_text(s_steps_layer, s_buf);
}
