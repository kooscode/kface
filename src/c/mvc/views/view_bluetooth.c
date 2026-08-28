#include "view_bluetooth.h"

// Small filled dot in the top-right corner - green while connected, red
// while not. Must match BORDER_THICKNESS in view_border.c so it sits just
// inside the border, same margin convention as view_battery's meter.
#define BORDER_THICKNESS 3
#define ICON_MARGIN 4
#define ICON_DIAMETER 8
// The layer is padded a couple px larger than the circle on every side -
// Pebble layers clip drawing to their own bounds by default, and a circle
// whose radius exactly reaches the edge of an equal-sized box can get its
// bottom/right rim clipped off by rasterization rounding. Padding keeps the
// whole circle safely inside the box.
#define ICON_PADDING 2
#define LAYER_SIZE (ICON_DIAMETER + 2 * ICON_PADDING)

static Layer *s_bluetooth_layer;
static bool s_connected;

static void bluetooth_update_proc(Layer *layer, GContext *ctx)
{
  GRect bounds = layer_get_bounds(layer);
  GPoint center = GPoint(bounds.size.w / 2, bounds.size.h / 2);

  graphics_context_set_fill_color(ctx, s_connected ? GColorGreen : GColorRed);
  graphics_fill_circle(ctx, center, ICON_DIAMETER / 2);
}

void view_bluetooth_create(Layer *parent, GRect bounds)
{
  GRect icon_bounds = GRect(
      bounds.size.w - BORDER_THICKNESS - ICON_MARGIN - LAYER_SIZE,
      BORDER_THICKNESS + ICON_MARGIN,
      LAYER_SIZE,
      LAYER_SIZE);
  s_bluetooth_layer = layer_create(icon_bounds);
  layer_set_update_proc(s_bluetooth_layer, bluetooth_update_proc);
  layer_add_child(parent, s_bluetooth_layer);

  view_bluetooth_set_connected(false);
}

void view_bluetooth_destroy(void)
{
  layer_destroy(s_bluetooth_layer);
}

void view_bluetooth_set_connected(bool connected)
{
  s_connected = connected;
  layer_mark_dirty(s_bluetooth_layer);
}
