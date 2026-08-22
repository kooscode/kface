#include "view_border.h"

#define BORDER_THICKNESS 3

static Layer *s_border_layer;

static void border_update_proc(Layer *layer, GContext *ctx)
{
  GRect bounds = layer_get_bounds(layer);

  // Fill the whole layer light blue, then punch out everything but a
  // BORDER_THICKNESS-wide ring back to black, leaving a light blue border.
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  GRect inner = grect_inset(bounds, GEdgeInsets(BORDER_THICKNESS));
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, inner, 0, GCornerNone);
}

void view_border_create(Layer *parent, GRect bounds)
{
  s_border_layer = layer_create(bounds);
  layer_set_update_proc(s_border_layer, border_update_proc);
  layer_add_child(parent, s_border_layer);
}

void view_border_destroy(void)
{
  layer_destroy(s_border_layer);
}
