#include "view_time.h"

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

// Row order top to bottom: time, then a gap left for view_battery's meter
// (see METER_Y/METER_HEIGHT in view_battery.c), then date. DATE_Y is tuned
// (not just mirrored off the box math) so the *rendered glyphs* end up the
// same visual distance from the meter on both sides - Roboto Bold Subset 49
// and Bitham 42 Bold have different top/bottom padding within their boxes,
// so equal box gaps alone don't produce equal visual gaps. Verified against
// an actual screenshot; re-check with one if these fonts/sizes change.
//
// TIME_Y is negative: Roboto Bold Subset 49 has ~14px of dead space baked
// into the font above the glyph's visible top, so a box literally flush
// with the border would still leave an ~14px visual gap. Pushing the box
// itself partway above y=0 moves that dead space off-screen while the
// glyph - which starts well inside the box - lands only ~4px below the
// border. Nothing visible is clipped: only the font's blank padding falls
// above row 0. TIME_HEIGHT/DATE_Y/DATE_HEIGHT here are the previous values
// shifted up by the same 14px so the whole time/battery/date group moves
// as one rigid block.
#define TIME_Y (-7)
#define TIME_HEIGHT 54
#define DATE_Y 58
#define DATE_HEIGHT 46

void view_time_create(Layer *parent, GRect bounds)
{
  s_time_layer = text_layer_create(GRect(0, TIME_Y, bounds.size.w, TIME_HEIGHT));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(parent, text_layer_get_layer(s_time_layer));

  s_date_layer = text_layer_create(GRect(0, DATE_Y, bounds.size.w, DATE_HEIGHT));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(parent, text_layer_get_layer(s_date_layer));
}

void view_time_destroy(void)
{
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
}

void view_time_set_time(const char *time_str)
{
  // TextLayer stores the pointer it's given, not a copy, so we keep our own
  // static buffer alive for as long as the layer might redraw.
  static char s_buf[10];
  snprintf(s_buf, sizeof(s_buf), "%s", time_str);
  text_layer_set_text(s_time_layer, s_buf);
}

void view_time_set_date(const char *date_str)
{
  static char s_buf[16];
  snprintf(s_buf, sizeof(s_buf), "%s", date_str);
  text_layer_set_text(s_date_layer, s_buf);
}
