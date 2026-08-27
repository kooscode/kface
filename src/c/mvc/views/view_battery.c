#include "view_battery.h"

// Shifted up 14px along with TIME_Y/DATE_Y in view_time.c to keep the
// time/battery/date group moving as one rigid block.
#define METER_Y 50
#define METER_HEIGHT 12
// Must match BORDER_THICKNESS in view_border.c - the meter spans the full
// width up to the inner edge of the border so its own edges touch it.
#define BORDER_THICKNESS 3

static Layer *s_battery_layer;
static uint8_t s_percent;

static void battery_update_proc(Layer *layer, GContext *ctx)
{
  GRect bounds = layer_get_bounds(layer);

  // 1px yellow border ring around the whole meter.
  graphics_context_set_fill_color(ctx, GColorYellow);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Blank (black) interior, then a green bar for the charge remaining,
  // growing from the left.
  GRect inner = grect_inset(bounds, GEdgeInsets(1));
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, inner, 0, GCornerNone);

  GRect charge = inner;
  charge.size.w = (inner.size.w * s_percent) / 100;

  //set the color of the charge bar based on the battery percentage
  if (s_percent < 15) {
    graphics_context_set_fill_color(ctx, GColorRed);
  } else if (s_percent < 25) {
    graphics_context_set_fill_color(ctx, GColorOrange);
  } else if (s_percent < 35) {
    graphics_context_set_fill_color(ctx, GColorYellow);
  } else {
    graphics_context_set_fill_color(ctx, GColorGreen);
  }

  graphics_fill_rect(ctx, charge, 0, GCornerNone);
}

void view_battery_create(Layer *parent, GRect bounds)
{
  GRect meter_bounds = GRect(BORDER_THICKNESS, METER_Y, bounds.size.w - 2 * BORDER_THICKNESS, METER_HEIGHT);
  s_battery_layer = layer_create(meter_bounds);
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(parent, s_battery_layer);

  view_battery_set_percent(0);
}

void view_battery_destroy(void)
{
  layer_destroy(s_battery_layer);
}

void view_battery_set_percent(uint8_t percent)
{
  s_percent = percent;
  layer_mark_dirty(s_battery_layer);
}
