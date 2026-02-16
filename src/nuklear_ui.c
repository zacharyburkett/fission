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
    struct nk_rect bounds;
    struct nk_rect track_rect;
    struct nk_color track_color;
    struct nk_color grip_color;
    float grip_half_span;
    float center_x;
    float center_y;
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
    bounds = nk_window_get_bounds(ctx);
    if (canvas == NULL || bounds.w <= 0.0f || bounds.h <= 0.0f) {
        nk_end(ctx);
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
        track_rect = bounds;
    } else {
        float thickness;

        thickness = fission_nk_clamp_float(bounds.h * 0.16f, 1.0f, 2.0f);
        track_rect = nk_rect(
            bounds.x,
            bounds.y + (bounds.h - thickness) * 0.5f,
            bounds.w,
            thickness
        );
    }
    nk_fill_rect(canvas, track_rect, 0.0f, track_color);

    center_x = bounds.x + bounds.w * 0.5f;
    center_y = bounds.y + bounds.h * 0.5f;
    grip_half_span = (vertical != 0) ? bounds.h * 0.16f : bounds.w * 0.16f;
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

    nk_end(ctx);
}
