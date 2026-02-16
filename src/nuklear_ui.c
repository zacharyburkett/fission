#include "fission/nuklear_ui.h"

#include <stdio.h>

#include "fission/nuklear_features.h"
#include "nuklear.h"

static float fission_nk_min_float(float a, float b)
{
    return (a < b) ? a : b;
}

static float fission_nk_max_float(float a, float b)
{
    return (a > b) ? a : b;
}

static float fission_nk_clamp_float(float value, float min_value, float max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

void fission_nk_apply_theme(struct nk_context *ctx)
{
    struct nk_color table[NK_COLOR_COUNT];

    if (ctx == NULL) {
        return;
    }

    table[NK_COLOR_TEXT] = nk_rgb(223, 228, 236);
    table[NK_COLOR_WINDOW] = nk_rgb(18, 23, 30);
    table[NK_COLOR_HEADER] = nk_rgb(33, 41, 52);
    table[NK_COLOR_BORDER] = nk_rgb(67, 81, 100);
    table[NK_COLOR_BUTTON] = nk_rgb(48, 59, 73);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgb(62, 77, 95);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgb(77, 94, 116);
    table[NK_COLOR_TOGGLE] = nk_rgb(51, 63, 79);
    table[NK_COLOR_TOGGLE_HOVER] = nk_rgb(63, 78, 96);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgb(162, 208, 189);
    table[NK_COLOR_SELECT] = nk_rgb(67, 90, 116);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgb(87, 120, 152);
    table[NK_COLOR_SLIDER] = nk_rgb(47, 58, 72);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgb(159, 205, 187);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgb(184, 224, 209);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgb(204, 236, 224);
    table[NK_COLOR_PROPERTY] = nk_rgb(40, 50, 64);
    table[NK_COLOR_EDIT] = nk_rgb(32, 40, 51);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgb(229, 236, 245);
    table[NK_COLOR_COMBO] = nk_rgb(33, 41, 53);
    table[NK_COLOR_CHART] = nk_rgb(40, 50, 64);
    table[NK_COLOR_CHART_COLOR] = nk_rgb(159, 205, 187);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgb(222, 170, 97);
    table[NK_COLOR_SCROLLBAR] = nk_rgb(30, 37, 47);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgb(71, 88, 108);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgb(86, 106, 129);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgb(104, 127, 155);
    table[NK_COLOR_TAB_HEADER] = nk_rgb(30, 37, 47);

    nk_style_from_table(ctx, table);

    ctx->style.window.border = 1.0f;
    ctx->style.window.rounding = 6.0f;
    ctx->style.window.spacing = nk_vec2(8.0f, 7.0f);
    ctx->style.window.padding = nk_vec2(10.0f, 10.0f);
    ctx->style.window.group_padding = nk_vec2(8.0f, 8.0f);
    ctx->style.window.min_row_height_padding = 2.0f;
    ctx->style.window.scrollbar_size = nk_vec2(13.0f, 13.0f);

    ctx->style.button.border = 1.0f;
    ctx->style.button.rounding = 5.0f;
    ctx->style.button.padding = nk_vec2(8.0f, 6.0f);
    ctx->style.button.text_alignment = NK_TEXT_CENTERED;

    ctx->style.combo.border = 1.0f;
    ctx->style.combo.rounding = 5.0f;
    ctx->style.combo.content_padding = nk_vec2(8.0f, 5.0f);
    ctx->style.combo.button_padding = nk_vec2(6.0f, 4.0f);
    ctx->style.combo.spacing = nk_vec2(5.0f, 5.0f);

    ctx->style.property.border = 1.0f;
    ctx->style.property.rounding = 5.0f;
    ctx->style.property.padding = nk_vec2(6.0f, 5.0f);

    ctx->style.edit.border = 1.0f;
    ctx->style.edit.rounding = 4.0f;
    ctx->style.edit.padding = nk_vec2(8.0f, 6.0f);
    ctx->style.edit.row_padding = 4.0f;
    ctx->style.edit.scrollbar_size = nk_vec2(10.0f, 10.0f);

    ctx->style.selectable.rounding = 4.0f;
    ctx->style.selectable.padding = nk_vec2(8.0f, 5.0f);
    ctx->style.selectable.text_alignment = NK_TEXT_LEFT;

    ctx->style.checkbox.spacing = 6.0f;
    ctx->style.option.spacing = 6.0f;

    ctx->style.scrollv.rounding = 6.0f;
    ctx->style.scrollv.rounding_cursor = 6.0f;
}

void fission_nk_make_window_id(
    char *buffer,
    size_t buffer_size,
    const char *window_id_prefix,
    const char *window_name
)
{
    if (buffer == NULL || buffer_size == 0u || window_name == NULL) {
        return;
    }

    if (window_id_prefix != NULL && window_id_prefix[0] != '\0') {
        (void)snprintf(buffer, buffer_size, "%s/%s", window_id_prefix, window_name);
    } else {
        (void)snprintf(buffer, buffer_size, "%s", window_name);
    }
}

void fission_nk_rect_translate(struct nk_rect *rect, float offset_x, float offset_y)
{
    if (rect == NULL) {
        return;
    }

    rect->x += offset_x;
    rect->y += offset_y;
}

void fission_nk_rect_intersection(
    const struct nk_rect *a,
    const struct nk_rect *b,
    struct nk_rect *out_rect
)
{
    float left;
    float top;
    float right;
    float bottom;

    if (a == NULL || b == NULL || out_rect == NULL) {
        return;
    }

    left = fission_nk_max_float(a->x, b->x);
    top = fission_nk_max_float(a->y, b->y);
    right = fission_nk_min_float(a->x + a->w, b->x + b->w);
    bottom = fission_nk_min_float(a->y + a->h, b->y + b->h);

    if (right <= left || bottom <= top) {
        *out_rect = nk_rect(0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }

    *out_rect = nk_rect(left, top, right - left, bottom - top);
}

static void fission_nk_draw_splitter(
    struct nk_command_buffer *canvas,
    const struct nk_rect *bounds,
    int vertical,
    int active,
    int hovered
)
{
    struct nk_rect track_rect;
    struct nk_color track_color;
    struct nk_color grip_color;
    float grip_half_span;
    float center_x;
    float center_y;

    if (canvas == NULL || bounds == NULL || bounds->w <= 0.0f || bounds->h <= 0.0f) {
        return;
    }

    if (active != 0) {
        track_color = nk_rgba(92, 108, 138, 220);
        grip_color = nk_rgba(220, 230, 255, 240);
    } else if (hovered != 0) {
        track_color = nk_rgba(80, 92, 116, 190);
        grip_color = nk_rgba(210, 218, 240, 220);
    } else {
        track_color = nk_rgba(65, 74, 92, 148);
        grip_color = nk_rgba(190, 200, 220, 165);
    }

    if (vertical != 0) {
        track_rect = *bounds;
    } else {
        float thickness;

        thickness = fission_nk_clamp_float(bounds->h * 0.16f, 1.0f, 2.0f);
        track_rect = nk_rect(
            bounds->x,
            bounds->y + (bounds->h - thickness) * 0.5f,
            bounds->w,
            thickness
        );
    }
    nk_fill_rect(canvas, track_rect, 0.0f, track_color);

    center_x = bounds->x + bounds->w * 0.5f;
    center_y = bounds->y + bounds->h * 0.5f;
    grip_half_span = (vertical != 0) ? bounds->h * 0.16f : bounds->w * 0.16f;
    grip_half_span = fission_nk_clamp_float(grip_half_span, 10.0f, 28.0f);

    if (vertical != 0) {
        float t0;
        float t1;

        t0 = center_y - grip_half_span;
        t1 = center_y + grip_half_span;
        nk_stroke_line(canvas, center_x - 2.0f, t0, center_x - 2.0f, t1, 1.0f, grip_color);
        nk_stroke_line(canvas, center_x, t0, center_x, t1, 1.0f, grip_color);
        nk_stroke_line(canvas, center_x + 2.0f, t0, center_x + 2.0f, t1, 1.0f, grip_color);
    } else {
        float t0;
        float t1;

        t0 = center_x - grip_half_span;
        t1 = center_x + grip_half_span;
        nk_stroke_line(canvas, t0, center_y - 0.5f, t1, center_y - 0.5f, 1.0f, grip_color);
        nk_stroke_line(canvas, t0, center_y, t1, center_y, 1.0f, grip_color);
        nk_stroke_line(canvas, t0, center_y + 0.5f, t1, center_y + 0.5f, 1.0f, grip_color);
    }
}

void fission_nk_draw_splitter_overlay(
    struct nk_context *ctx,
    const char *name,
    const struct nk_rect *rect,
    int vertical,
    int active,
    int hovered
)
{
    struct nk_command_buffer *canvas;
    nk_flags flags;

    if (
        ctx == NULL ||
        name == NULL ||
        rect == NULL ||
        rect->w <= 0.0f ||
        rect->h <= 0.0f
    ) {
        return;
    }

    flags = NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_NO_INPUT | NK_WINDOW_BACKGROUND;
    if (!nk_begin(ctx, name, *rect, flags)) {
        nk_end(ctx);
        return;
    }

    canvas = nk_window_get_canvas(ctx);
    if (canvas == NULL) {
        nk_end(ctx);
        return;
    }

    fission_nk_draw_splitter(canvas, rect, vertical, active, hovered);

    nk_end(ctx);
}

int fission_nk_begin_titled_group_in_space(
    struct nk_context *ctx,
    const char *group_id,
    const char *title,
    const struct nk_rect *bounds,
    unsigned int flags
)
{
    struct nk_rect resolved_bounds;

    if (
        ctx == NULL ||
        group_id == NULL ||
        title == NULL ||
        bounds == NULL
    ) {
        return 0;
    }

    resolved_bounds = *bounds;
    if (resolved_bounds.w <= 0.0f) {
        resolved_bounds.w = 1.0f;
    }
    if (resolved_bounds.h <= 0.0f) {
        resolved_bounds.h = 1.0f;
    }

    nk_layout_space_push(ctx, resolved_bounds);
    return nk_group_begin_titled(ctx, group_id, title, (nk_flags)flags);
}

void fission_nk_draw_splitter_canvas(
    struct nk_context *ctx,
    const struct nk_rect *rect,
    int vertical,
    int active,
    int hovered
)
{
    struct nk_command_buffer *canvas;

    if (ctx == NULL || rect == NULL || rect->w <= 0.0f || rect->h <= 0.0f) {
        return;
    }

    canvas = nk_window_get_canvas(ctx);
    if (canvas == NULL) {
        return;
    }

    fission_nk_draw_splitter(canvas, rect, vertical, active, hovered);
}

int fission_nk_update_splitter_interaction(
    struct nk_context *ctx,
    const struct nk_rect *rect,
    int vertical,
    int splitter_id,
    int *active_splitter_id,
    int *hovered_splitter_id,
    float *out_delta
)
{
    int hovered;

    if (
        ctx == NULL ||
        rect == NULL ||
        active_splitter_id == NULL ||
        rect->w <= 0.0f ||
        rect->h <= 0.0f
    ) {
        return 0;
    }

    if (out_delta != NULL) {
        *out_delta = 0.0f;
    }

    hovered = nk_input_is_mouse_hovering_rect(&ctx->input, *rect);
    if (
        hovered != 0 &&
        hovered_splitter_id != NULL &&
        (*active_splitter_id == 0 || *active_splitter_id == splitter_id)
    ) {
        *hovered_splitter_id = splitter_id;
    }

    if (*active_splitter_id == splitter_id) {
        if (nk_input_is_mouse_down(&ctx->input, NK_BUTTON_LEFT)) {
            if (out_delta != NULL) {
                *out_delta = (vertical != 0) ? ctx->input.mouse.delta.x : ctx->input.mouse.delta.y;
            }
            return 1;
        }

        *active_splitter_id = 0;
        return 0;
    }

    if (
        hovered != 0 &&
        nk_input_is_mouse_pressed(&ctx->input, NK_BUTTON_LEFT)
    ) {
        *active_splitter_id = splitter_id;
        if (out_delta != NULL) {
            *out_delta = (vertical != 0) ? ctx->input.mouse.delta.x : ctx->input.mouse.delta.y;
        }
        return 1;
    }

    return 0;
}

void fission_nk_build_dock_zones(
    const struct nk_rect *bounds,
    float edge_fraction,
    float min_edge_size,
    struct nk_rect *out_zones
)
{
    size_t i;
    float edge_x;
    float edge_y;
    float inner_x;
    float inner_y;
    float inner_w;
    float inner_h;

    if (out_zones == NULL) {
        return;
    }

    for (i = 0u; i < (size_t)FISSION_NK_DOCK_ZONE_COUNT; ++i) {
        out_zones[i] = nk_rect(0.0f, 0.0f, 0.0f, 0.0f);
    }

    if (bounds == NULL || bounds->w <= 0.0f || bounds->h <= 0.0f) {
        return;
    }

    edge_fraction = fission_nk_clamp_float(edge_fraction, 0.12f, 0.38f);
    min_edge_size = fission_nk_clamp_float(min_edge_size, 16.0f, 320.0f);

    edge_x = fission_nk_max_float(bounds->w * edge_fraction, min_edge_size);
    edge_y = fission_nk_max_float(bounds->h * edge_fraction, min_edge_size);
    edge_x = fission_nk_min_float(edge_x, bounds->w * 0.42f);
    edge_y = fission_nk_min_float(edge_y, bounds->h * 0.42f);

    inner_x = bounds->x + edge_x;
    inner_y = bounds->y + edge_y;
    inner_w = bounds->w - edge_x * 2.0f;
    inner_h = bounds->h - edge_y * 2.0f;
    if (inner_w < 1.0f) {
        inner_w = 1.0f;
    }
    if (inner_h < 1.0f) {
        inner_h = 1.0f;
    }

    out_zones[FISSION_NK_DOCK_ZONE_LEFT] = nk_rect(
        bounds->x,
        inner_y,
        edge_x,
        inner_h
    );
    out_zones[FISSION_NK_DOCK_ZONE_RIGHT] = nk_rect(
        bounds->x + bounds->w - edge_x,
        inner_y,
        edge_x,
        inner_h
    );
    out_zones[FISSION_NK_DOCK_ZONE_TOP] = nk_rect(
        inner_x,
        bounds->y,
        inner_w,
        edge_y
    );
    out_zones[FISSION_NK_DOCK_ZONE_BOTTOM] = nk_rect(
        inner_x,
        bounds->y + bounds->h - edge_y,
        inner_w,
        edge_y
    );
    out_zones[FISSION_NK_DOCK_ZONE_CENTER] = nk_rect(
        inner_x,
        inner_y,
        inner_w,
        inner_h
    );
}

static int fission_nk_point_in_rect(
    const struct nk_rect *rect,
    float x,
    float y
)
{
    if (rect == NULL || rect->w <= 0.0f || rect->h <= 0.0f) {
        return 0;
    }

    return (
        x >= rect->x &&
        y >= rect->y &&
        x <= (rect->x + rect->w) &&
        y <= (rect->y + rect->h)
    );
}

fission_nk_dock_zone_t fission_nk_pick_dock_zone(
    const struct nk_rect *zones,
    float x,
    float y
)
{
    if (zones == NULL) {
        return FISSION_NK_DOCK_ZONE_NONE;
    }

    if (fission_nk_point_in_rect(&zones[FISSION_NK_DOCK_ZONE_LEFT], x, y) != 0) {
        return FISSION_NK_DOCK_ZONE_LEFT;
    }
    if (fission_nk_point_in_rect(&zones[FISSION_NK_DOCK_ZONE_RIGHT], x, y) != 0) {
        return FISSION_NK_DOCK_ZONE_RIGHT;
    }
    if (fission_nk_point_in_rect(&zones[FISSION_NK_DOCK_ZONE_TOP], x, y) != 0) {
        return FISSION_NK_DOCK_ZONE_TOP;
    }
    if (fission_nk_point_in_rect(&zones[FISSION_NK_DOCK_ZONE_BOTTOM], x, y) != 0) {
        return FISSION_NK_DOCK_ZONE_BOTTOM;
    }
    if (fission_nk_point_in_rect(&zones[FISSION_NK_DOCK_ZONE_CENTER], x, y) != 0) {
        return FISSION_NK_DOCK_ZONE_CENTER;
    }

    return FISSION_NK_DOCK_ZONE_NONE;
}

void fission_nk_draw_dock_zones_overlay(
    struct nk_context *ctx,
    const char *name,
    const struct nk_rect *bounds,
    const struct nk_rect *zones,
    fission_nk_dock_zone_t hovered_zone
)
{
    struct nk_command_buffer *canvas;
    nk_flags flags;
    size_t i;

    if (ctx == NULL || name == NULL || bounds == NULL || zones == NULL) {
        return;
    }
    if (bounds->w <= 0.0f || bounds->h <= 0.0f) {
        return;
    }

    flags = NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_NO_INPUT | NK_WINDOW_BACKGROUND;
    if (!nk_begin(ctx, name, *bounds, flags)) {
        nk_end(ctx);
        return;
    }

    canvas = nk_window_get_canvas(ctx);
    if (canvas == NULL) {
        nk_end(ctx);
        return;
    }

    for (i = 0u; i < (size_t)FISSION_NK_DOCK_ZONE_COUNT; ++i) {
        struct nk_color fill;
        struct nk_color border;
        int active;

        if (zones[i].w <= 0.0f || zones[i].h <= 0.0f) {
            continue;
        }

        active = ((int)i == (int)hovered_zone);
        if (active != 0) {
            fill = nk_rgba(90, 124, 176, 132);
            border = nk_rgba(208, 226, 255, 212);
        } else {
            fill = nk_rgba(60, 76, 104, 92);
            border = nk_rgba(140, 162, 198, 138);
        }

        nk_fill_rect(canvas, zones[i], 7.0f, fill);
        nk_stroke_rect(canvas, zones[i], 7.0f, 1.0f, border);
    }

    nk_end(ctx);
}
