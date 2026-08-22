#include "view_weather.h"

// Sits in the blank space between view_dividers' two lines (roughly rows
// 110-188 at the current layout) - see LINE1_Y/LINE2_Y in view_dividers.c.
// Temp (left half) + condition (right, between temp and the border) share
// one row, city sits below.
//
// Must match BORDER_THICKNESS in view_border.c - condition's box stops at
// the border's inner edge instead of the screen edge, same as the meter/
// dividers.
#define BORDER_THICKNESS 3

#define TEMP_Y 116
#define TEMP_HEIGHT 40
// CONDITION_Y is TEMP_Y shifted so the two *glyphs'* vertical centers line
// up - Bitham 34 Medium Numbers and Gothic 28 Bold have different top
// padding within their boxes, so equal box positions alone don't produce
// equal visual centers (same issue as DATE_Y in view_time.c). Measured off
// an actual screenshot; re-check if these fonts/sizes change.
#define CONDITION_Y (TEMP_Y + 3)
#define CONDITION_HEIGHT 40
#define TEMP_FONT FONT_KEY_BITHAM_34_MEDIUM_NUMBERS
// Likewise tuned against a screenshot so the city glyph sits vertically
// centered between the bottom of the temp/condition row and view_dividers'
// LINE2_Y, not just box-centered.
#define CITY_Y (TEMP_Y + 34)
#define CITY_HEIGHT 28

static TextLayer *s_temp_layer;
static TextLayer *s_condition_layer;
static TextLayer *s_city_layer;
static GRect s_bounds;

// Open-Meteo/WMO weather codes, grouped into short display labels - kept
// short since condition now shares a half-width row with the temperature.
static const char *condition_text(uint8_t code)
{
  switch (code) {
    case 0:
    case 1:
      return "Clear";
    case 2:
      return "Cloudy";
    case 3:
      return "Overcast";
    case 45:
    case 48:
      return "Fog";
    case 51:
    case 53:
    case 55:
      return "Drizzle";
    case 56:
    case 57:
      return "Icy Drzl";
    case 61:
    case 63:
    case 65:
      return "Rain";
    case 66:
    case 67:
      return "Icy Rain";
    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86:
      return "Snow";
    case 80:
    case 81:
    case 82:
      return "Showers";
    case 95:
    case 96:
    case 99:
      return "Storm";
    default:
      return "--";
  }
}

// Condition's box is recomputed (not fixed at creation) so it's centered
// between temp's *rendered glyph* and the border, not just temp's column -
// temp's text width varies (e.g. "-5°F" vs "104°F"), so a static box would
// only be centered for whichever value it was measured against.
static void reposition_condition(void)
{
  GFont font = fonts_get_system_font(TEMP_FONT);
  GRect measure_box = GRect(0, 0, s_bounds.size.w / 2, TEMP_HEIGHT);
  GSize temp_size = graphics_text_layout_get_content_size(
      text_layer_get_text(s_temp_layer), font, measure_box, GTextOverflowModeFill, GTextAlignmentLeft);

  // temp's box is [0, bounds.w/2] with centered text, so its glyph is
  // centered on bounds.w/4 - from there we can get the glyph's right edge
  // without needing the box's own internal left/right padding.
  int16_t temp_glyph_right = (s_bounds.size.w / 4) + (temp_size.w / 2);
  int16_t border_right = s_bounds.size.w - BORDER_THICKNESS;

  GRect frame = GRect(temp_glyph_right, CONDITION_Y, border_right - temp_glyph_right, CONDITION_HEIGHT);
  layer_set_frame(text_layer_get_layer(s_condition_layer), frame);
}

void view_weather_create(Layer *parent, GRect bounds)
{
  s_bounds = bounds;

  s_temp_layer = text_layer_create(GRect(0, TEMP_Y, bounds.size.w / 2, TEMP_HEIGHT));
  text_layer_set_background_color(s_temp_layer, GColorClear);
  text_layer_set_text_color(s_temp_layer, GColorWhite);
  // Same font size as HR/steps, per request.
  text_layer_set_font(s_temp_layer, fonts_get_system_font(TEMP_FONT));
  text_layer_set_text_alignment(s_temp_layer, GTextAlignmentCenter);
  layer_add_child(parent, text_layer_get_layer(s_temp_layer));

  // Frame is a placeholder - reposition_condition() (called from
  // view_weather_set_temp()) places it based on temp's actual glyph width.
  s_condition_layer = text_layer_create(GRect(bounds.size.w / 2, CONDITION_Y, bounds.size.w / 2, CONDITION_HEIGHT));
  text_layer_set_background_color(s_condition_layer, GColorClear);
  text_layer_set_text_color(s_condition_layer, GColorPictonBlue);
  text_layer_set_font(s_condition_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_condition_layer, GTextAlignmentCenter);
  layer_add_child(parent, text_layer_get_layer(s_condition_layer));

  s_city_layer = text_layer_create(GRect(0, CITY_Y, bounds.size.w, CITY_HEIGHT));
  text_layer_set_background_color(s_city_layer, GColorClear);
  text_layer_set_text_color(s_city_layer, GColorPictonBlue);
  text_layer_set_font(s_city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_city_layer, GTextAlignmentCenter);
  layer_add_child(parent, text_layer_get_layer(s_city_layer));

  view_weather_set_temp(false, 0);
  view_weather_set_condition(false, 0);
  view_weather_set_location("");
}

void view_weather_destroy(void)
{
  text_layer_destroy(s_temp_layer);
  text_layer_destroy(s_condition_layer);
  text_layer_destroy(s_city_layer);
}

void view_weather_set_temp(bool valid, int16_t temp_c_tenths)
{
  static char s_buf[16];
  if (!valid) {
    snprintf(s_buf, sizeof(s_buf), "--°");
  } else {
    // Wire format is Celsius tenths (see model.h) - PebbleKit JS sends what
    // Open-Meteo returns without doing unit math on the phone. Convert to
    // Fahrenheit and round to the nearest whole degree here instead.
    int temp_f_tenths = (temp_c_tenths * 9) / 5 + 320;
    int whole = (temp_f_tenths >= 0) ? (temp_f_tenths + 5) / 10 : -((-temp_f_tenths + 5) / 10);
    snprintf(s_buf, sizeof(s_buf), "%d°F", whole);
  }
  text_layer_set_text(s_temp_layer, s_buf);
  reposition_condition();
}

void view_weather_set_condition(bool valid, uint8_t condition_code)
{
  text_layer_set_text(s_condition_layer, valid ? condition_text(condition_code) : "--");
}

void view_weather_set_location(const char *city)
{
  text_layer_set_text(s_city_layer, (city && city[0]) ? city : "--");
}
