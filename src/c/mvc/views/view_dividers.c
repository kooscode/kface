#include "view_dividers.h"

// Must match BORDER_THICKNESS in view_border.c - both lines span the full
// width up to the inner edge of the border, same as view_battery's meter.
#define BORDER_THICKNESS 3
#define LINE_THICKNESS 2

// Both gaps and glyph positions below are measured off an actual screenshot
// (see view_time.c's DATE_Y comment for why box math alone isn't enough).
// TOP_GAP: rows between the date glyphs and line1. BOTTOM_GAP: rows between
// line2 and the top of the HR/steps glyphs. DATE_GLYPH_BOTTOM/ROW_GLYPH_TOP:
// measured glyph edges of the date text and the HR/steps row, after the 14px
// upward shift in view_time.c and the 7px downward shift in view_heart.c /
// view_steps.c. Re-check against a screenshot if fonts, row heights, or
// border thickness change.
#define TOP_GAP 11
#define BOTTOM_GAP 6
#define DATE_GLYPH_BOTTOM 103
#define ROW_GLYPH_TOP 195

// As far below the date as the requested ~4px gap.
#define LINE1_Y (DATE_GLYPH_BOTTOM + 1 + TOP_GAP)
// As far above the HR/steps row as the requested ~4px gap.
#define LINE2_Y (ROW_GLYPH_TOP - BOTTOM_GAP - LINE_THICKNESS)

static Layer *s_dividers_layer;

static void dividers_update_proc(Layer *layer, GContext *ctx)
{
  GRect bounds = layer_get_bounds(layer);
  GRect line1 = GRect(BORDER_THICKNESS, LINE1_Y, bounds.size.w - 2 * BORDER_THICKNESS, LINE_THICKNESS);
  GRect line2 = GRect(BORDER_THICKNESS, LINE2_Y, bounds.size.w - 2 * BORDER_THICKNESS, LINE_THICKNESS);

  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  graphics_fill_rect(ctx, line1, 0, GCornerNone);

  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  graphics_fill_rect(ctx, line2, 0, GCornerNone);
}

void view_dividers_create(Layer *parent, GRect bounds)
{
  s_dividers_layer = layer_create(bounds);
  layer_set_update_proc(s_dividers_layer, dividers_update_proc);
  layer_add_child(parent, s_dividers_layer);
}

void view_dividers_destroy(void)
{
  layer_destroy(s_dividers_layer);
}
