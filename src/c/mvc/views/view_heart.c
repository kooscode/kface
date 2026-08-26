#include "view_heart.h"

static TextLayer *s_heart_layer;

// Bottom row, left half - see view_steps.c for the matching right half.
// Shifted down 7px so the row's rendered glyphs end up ~4px above the
// bottom border (was ~11px) - see the ROW_GLYPH_TOP/BOTTOM_GAP comment in
// view_dividers.c for how that was measured.
#define ROW_Y 186
#define ROW_HEIGHT 44

void view_heart_create(Layer *parent, GRect bounds)
{
  s_heart_layer = text_layer_create(GRect(0, ROW_Y, bounds.size.w / 2, ROW_HEIGHT));
  text_layer_set_background_color(s_heart_layer, GColorClear);
  text_layer_set_text_color(s_heart_layer, GColorYellow);
  text_layer_set_font(s_heart_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
  text_layer_set_text_alignment(s_heart_layer, GTextAlignmentLeft);
  layer_add_child(parent, text_layer_get_layer(s_heart_layer));

  view_heart_set_bpm(0);
}

void view_heart_destroy(void)
{
  text_layer_destroy(s_heart_layer);
}

void view_heart_set_bpm(uint32_t bpm)
{
  // Set the color of the heart rate text based on the bpm value
  if (bpm < 110)
    text_layer_set_text_color(s_heart_layer, GColorGreen);
  else if (bpm < 140) 
    text_layer_set_text_color(s_heart_layer, GColorYellow);
  else
    text_layer_set_text_color(s_heart_layer, GColorRed);  

  static char s_buf[16];
  if (bpm == 0)
  {
    snprintf(s_buf, sizeof(s_buf), "💛 --");
  }
  else
  {
    snprintf(s_buf, sizeof(s_buf), "💛 %lu", bpm);
  }
  text_layer_set_text(s_heart_layer, s_buf);
}
