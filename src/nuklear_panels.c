#include "fission/nuklear_panels.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "fission/nuklear_features.h"
#include "fission/nuklear_ui.h"
#include "nuklear.h"

#define FISSION_NK_PANEL_MARGIN 12.0f
#define FISSION_NK_PANEL_TOP_RESERVED 34.0f
#define FISSION_NK_PANEL_GAP 10.0f
#define FISSION_NK_PANEL_MIN_LEFT_WIDTH 220.0f
#define FISSION_NK_PANEL_MIN_CENTER_WIDTH 320.0f
#define FISSION_NK_PANEL_MIN_RIGHT_WIDTH 260.0f
#define FISSION_NK_PANEL_MIN_TOP_HEIGHT 120.0f
#define FISSION_NK_PANEL_MIN_BOTTOM_HEIGHT 120.0f
#define FISSION_NK_PANEL_MIN_HEIGHT 120.0f
#define FISSION_NK_PANEL_DOCK_EDGE_FRACTION 0.24f
#define FISSION_NK_PANEL_DOCK_MIN_EDGE_SIZE 110.0f

enum {
    FISSION_NK_PANEL_SPLITTER_NONE = 0,
    FISSION_NK_PANEL_SPLITTER_LEFT = 1,
    FISSION_NK_PANEL_SPLITTER_RIGHT = 2,
    FISSION_NK_PANEL_SPLITTER_TOP = 3,
    FISSION_NK_PANEL_SPLITTER_BOTTOM = 4
};

static void fission_nk_panel_overlay_id(
    char *buffer,
    size_t buffer_size,
    const fission_nk_panel_workspace_t *workspace,
    const char *suffix
)
{
    if (
        buffer == NULL ||
        buffer_size == 0u ||
        workspace == NULL ||
        suffix == NULL
    ) {
        return;
    }

    (void)snprintf(
        buffer,
        buffer_size,
        "__fission_panels_%p_%s",
        (const void *)workspace,
        suffix
    );
}

static float fission_nk_panel_clamp_float(float value, float min_value, float max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static void fission_nk_panel_bounds_zero(fission_nk_panel_bounds_t *bounds)
{
    if (bounds == NULL) {
        return;
    }

    bounds->x = 0.0f;
    bounds->y = 0.0f;
    bounds->w = 0.0f;
    bounds->h = 0.0f;
}

static struct nk_rect fission_nk_panel_bounds_to_nk_rect(const fission_nk_panel_bounds_t *bounds)
{
    if (bounds == NULL) {
        return nk_rect(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return nk_rect(bounds->x, bounds->y, bounds->w, bounds->h);
}

static int fission_nk_panel_point_in_bounds(
    const fission_nk_panel_bounds_t *bounds,
    float x,
    float y
)
{
    if (bounds == NULL || bounds->w <= 0.0f || bounds->h <= 0.0f) {
        return 0;
    }

    return (
        x >= bounds->x &&
        y >= bounds->y &&
        x <= bounds->x + bounds->w &&
        y <= bounds->y + bounds->h
    );
}

static fission_nk_panel_bounds_t fission_nk_panel_header_button_bounds(
    const fission_nk_panel_bounds_t *window_bounds
)
{
    fission_nk_panel_bounds_t bounds;

    bounds.x = 0.0f;
    bounds.y = 0.0f;
    bounds.w = 0.0f;
    bounds.h = 0.0f;
    if (
        window_bounds == NULL ||
        window_bounds->w <= 0.0f ||
        window_bounds->h <= 0.0f
    ) {
        return bounds;
    }

    bounds.w = FISSION_NK_PANEL_HEADER_BUTTON_WIDTH;
    bounds.h = FISSION_NK_PANEL_HEADER_BUTTON_HEIGHT;

    if (window_bounds->w < bounds.w + FISSION_NK_PANEL_HEADER_BUTTON_MARGIN * 2.0f) {
        bounds.w = window_bounds->w - FISSION_NK_PANEL_HEADER_BUTTON_MARGIN * 2.0f;
    }
    if (bounds.w < 1.0f) {
        bounds.w = 1.0f;
    }

    bounds.x = window_bounds->x + window_bounds->w - bounds.w - FISSION_NK_PANEL_HEADER_BUTTON_MARGIN;
    bounds.y = window_bounds->y + (FISSION_NK_PANEL_TITLE_BAR_HEIGHT - bounds.h) * 0.5f;
    if (bounds.y < window_bounds->y + 1.0f) {
        bounds.y = window_bounds->y + 1.0f;
    }
    if (bounds.h > FISSION_NK_PANEL_TITLE_BAR_HEIGHT - 2.0f) {
        bounds.h = FISSION_NK_PANEL_TITLE_BAR_HEIGHT - 2.0f;
    }
    if (bounds.h < 1.0f) {
        bounds.h = 1.0f;
    }

    return bounds;
}

static size_t fission_nk_panel_find_index(
    const fission_nk_panel_workspace_t *host,
    const char *panel_id
)
{
    size_t i;

    if (host == NULL || panel_id == NULL) {
        return (size_t)FISSION_NK_MAX_PANELS;
    }

    for (i = 0u; i < host->count; ++i) {
        if (strcmp(host->entries[i].desc.id, panel_id) == 0) {
            return i;
        }
    }

    return (size_t)FISSION_NK_MAX_PANELS;
}

static fission_nk_panel_bounds_t fission_nk_panel_default_detached_bounds(
    fission_nk_panel_slot_t slot
)
{
    fission_nk_panel_bounds_t bounds;

    bounds.x = 80.0f;
    bounds.y = 80.0f;
    bounds.w = 560.0f;
    bounds.h = 420.0f;

    if (slot == FISSION_NK_PANEL_SLOT_LEFT) {
        bounds.x = 40.0f;
        bounds.y = 40.0f;
        bounds.w = 500.0f;
        bounds.h = 760.0f;
    } else if (slot == FISSION_NK_PANEL_SLOT_RIGHT) {
        bounds.x = 980.0f;
        bounds.y = 40.0f;
        bounds.w = 420.0f;
        bounds.h = 760.0f;
    } else if (slot == FISSION_NK_PANEL_SLOT_TOP) {
        bounds.x = 120.0f;
        bounds.y = 40.0f;
        bounds.w = 1020.0f;
        bounds.h = 320.0f;
    } else if (slot == FISSION_NK_PANEL_SLOT_BOTTOM) {
        bounds.x = 120.0f;
        bounds.y = 520.0f;
        bounds.w = 1020.0f;
        bounds.h = 320.0f;
    }

    return bounds;
}

static void fission_nk_panel_sanitize_detached_bounds(
    fission_nk_panel_workspace_t *host,
    fission_nk_panel_bounds_t *bounds
)
{
    float max_width;
    float max_height;
    float min_x;
    float min_y;
    float max_x;
    float max_y;
    float min_width;
    float min_height;

    if (host == NULL || bounds == NULL) {
        return;
    }

    min_width = 240.0f;
    min_height = 180.0f;
    max_width = (float)host->last_window_width - FISSION_NK_PANEL_MARGIN * 2.0f;
    max_height = (float)host->last_window_height - FISSION_NK_PANEL_MARGIN * 2.0f;

    if (max_width < 1.0f) {
        max_width = 1.0f;
    }
    if (max_height < 1.0f) {
        max_height = 1.0f;
    }

    bounds->w = fission_nk_panel_clamp_float(bounds->w, min_width, max_width);
    bounds->h = fission_nk_panel_clamp_float(bounds->h, min_height, max_height);

    min_x = FISSION_NK_PANEL_MARGIN;
    min_y = FISSION_NK_PANEL_MARGIN;
    max_x = (float)host->last_window_width - FISSION_NK_PANEL_MARGIN - bounds->w;
    max_y = (float)host->last_window_height - FISSION_NK_PANEL_MARGIN - bounds->h;

    if (max_x < min_x) {
        max_x = min_x;
    }
    if (max_y < min_y) {
        max_y = min_y;
    }

    bounds->x = fission_nk_panel_clamp_float(bounds->x, min_x, max_x);
    bounds->y = fission_nk_panel_clamp_float(bounds->y, min_y, max_y);
}

static float fission_nk_panel_take_extent(float *value, float min_value, float needed)
{
    float available;
    float taken;

    if (value == NULL || needed <= 0.0f) {
        return needed;
    }

    available = *value - min_value;
    if (available <= 0.0f) {
        return needed;
    }

    if (available < needed) {
        taken = available;
    } else {
        taken = needed;
    }

    *value -= taken;
    return needed - taken;
}

static void fission_nk_panel_layout_stack_vertical(
    fission_nk_panel_workspace_t *host,
    const size_t *indices,
    size_t count,
    float x,
    float y,
    float w,
    float h
)
{
    float available_height;
    float panel_height;
    float cursor_y;
    size_t i;

    if (
        host == NULL ||
        indices == NULL ||
        count == 0u ||
        w <= 0.0f ||
        h <= 0.0f
    ) {
        return;
    }

    available_height = h - FISSION_NK_PANEL_GAP * (float)(count - 1u);
    if (available_height < 1.0f) {
        available_height = 1.0f;
    }

    panel_height = available_height / (float)count;
    if (panel_height < 1.0f) {
        panel_height = 1.0f;
    }

    cursor_y = y;
    for (i = 0u; i < count; ++i) {
        fission_nk_panel_state_t *state;

        state = &host->entries[indices[i]].state;
        state->resolved_bounds.x = x;
        state->resolved_bounds.y = cursor_y;
        state->resolved_bounds.w = w;
        state->resolved_bounds.h = panel_height;

        cursor_y += panel_height + FISSION_NK_PANEL_GAP;
    }
}

static void fission_nk_panel_layout_stack_horizontal(
    fission_nk_panel_workspace_t *host,
    const size_t *indices,
    size_t count,
    float x,
    float y,
    float w,
    float h
)
{
    float available_width;
    float panel_width;
    float cursor_x;
    size_t i;

    if (
        host == NULL ||
        indices == NULL ||
        count == 0u ||
        w <= 0.0f ||
        h <= 0.0f
    ) {
        return;
    }

    available_width = w - FISSION_NK_PANEL_GAP * (float)(count - 1u);
    if (available_width < 1.0f) {
        available_width = 1.0f;
    }

    panel_width = available_width / (float)count;
    if (panel_width < 1.0f) {
        panel_width = 1.0f;
    }

    cursor_x = x;
    for (i = 0u; i < count; ++i) {
        fission_nk_panel_state_t *state;

        state = &host->entries[indices[i]].state;
        state->resolved_bounds.x = cursor_x;
        state->resolved_bounds.y = y;
        state->resolved_bounds.w = panel_width;
        state->resolved_bounds.h = h;
        cursor_x += panel_width + FISSION_NK_PANEL_GAP;
    }
}

static fission_nk_panel_slot_t fission_nk_panel_slot_from_dock_zone(
    fission_nk_dock_zone_t zone
)
{
    if (zone == FISSION_NK_DOCK_ZONE_LEFT) {
        return FISSION_NK_PANEL_SLOT_LEFT;
    }
    if (zone == FISSION_NK_DOCK_ZONE_RIGHT) {
        return FISSION_NK_PANEL_SLOT_RIGHT;
    }
    if (zone == FISSION_NK_DOCK_ZONE_TOP) {
        return FISSION_NK_PANEL_SLOT_TOP;
    }
    if (zone == FISSION_NK_DOCK_ZONE_BOTTOM) {
        return FISSION_NK_PANEL_SLOT_BOTTOM;
    }
    return FISSION_NK_PANEL_SLOT_CENTER;
}

static fission_nk_dock_zone_t fission_nk_panel_dock_zone_from_slot(
    fission_nk_panel_slot_t slot
)
{
    if (slot == FISSION_NK_PANEL_SLOT_LEFT) {
        return FISSION_NK_DOCK_ZONE_LEFT;
    }
    if (slot == FISSION_NK_PANEL_SLOT_RIGHT) {
        return FISSION_NK_DOCK_ZONE_RIGHT;
    }
    if (slot == FISSION_NK_PANEL_SLOT_TOP) {
        return FISSION_NK_DOCK_ZONE_TOP;
    }
    if (slot == FISSION_NK_PANEL_SLOT_BOTTOM) {
        return FISSION_NK_DOCK_ZONE_BOTTOM;
    }
    return FISSION_NK_DOCK_ZONE_CENTER;
}

static int fission_nk_panel_host_resolve_layout(
    fission_nk_panel_workspace_t *host,
    int window_width,
    int window_height
)
{
    size_t left_indices[FISSION_NK_MAX_PANELS];
    size_t center_indices[FISSION_NK_MAX_PANELS];
    size_t right_indices[FISSION_NK_MAX_PANELS];
    size_t top_indices[FISSION_NK_MAX_PANELS];
    size_t bottom_indices[FISSION_NK_MAX_PANELS];
    size_t left_count;
    size_t center_count;
    size_t right_count;
    size_t top_count;
    size_t bottom_count;
    size_t i;
    int has_left;
    int has_center;
    int has_right;
    int has_top;
    int has_bottom;
    int has_mid;
    int vertical_gutters;
    int horizontal_gutters;
    float content_x;
    float content_y;
    float content_w;
    float content_h;
    float shared_h;
    float top_h;
    float mid_h;
    float bottom_h;
    float top_min;
    float mid_min;
    float bottom_min;
    float deficit;
    float cursor_y;
    float top_y;
    float mid_y;
    float bottom_y;
    float shared_w;
    float left_w;
    float center_w;
    float right_w;
    float left_min;
    float center_min;
    float right_min;
    float cursor_x;
    float left_x;
    float center_x;
    float right_x;

    if (host == NULL) {
        return 0;
    }

    host->last_window_width = window_width;
    host->last_window_height = window_height;
    fission_nk_panel_bounds_zero(&host->splitter_left_bounds);
    fission_nk_panel_bounds_zero(&host->splitter_right_bounds);
    fission_nk_panel_bounds_zero(&host->splitter_top_bounds);
    fission_nk_panel_bounds_zero(&host->splitter_bottom_bounds);

    content_x = FISSION_NK_PANEL_MARGIN;
    content_y = FISSION_NK_PANEL_MARGIN + FISSION_NK_PANEL_TOP_RESERVED;
    content_w = (float)window_width - FISSION_NK_PANEL_MARGIN * 2.0f;
    content_h = (float)window_height - FISSION_NK_PANEL_MARGIN - content_y;
    if (content_w < 1.0f) {
        content_w = 1.0f;
    }
    if (content_h < 1.0f) {
        content_h = 1.0f;
    }
    host->dock_workspace_bounds.x = content_x;
    host->dock_workspace_bounds.y = content_y;
    host->dock_workspace_bounds.w = content_w;
    host->dock_workspace_bounds.h = content_h;

    left_count = 0u;
    center_count = 0u;
    right_count = 0u;
    top_count = 0u;
    bottom_count = 0u;

    for (i = 0u; i < host->count; ++i) {
        fission_nk_panel_entry_t *entry;

        entry = &host->entries[i];
        if (entry->state.visible == 0) {
            continue;
        }

        if (entry->state.detached != 0) {
            fission_nk_panel_sanitize_detached_bounds(host, &entry->state.detached_bounds);
            entry->state.resolved_bounds = entry->state.detached_bounds;
            continue;
        }

        if (entry->state.slot == FISSION_NK_PANEL_SLOT_LEFT) {
            left_indices[left_count] = i;
            left_count += 1u;
        } else if (entry->state.slot == FISSION_NK_PANEL_SLOT_RIGHT) {
            right_indices[right_count] = i;
            right_count += 1u;
        } else if (entry->state.slot == FISSION_NK_PANEL_SLOT_TOP) {
            top_indices[top_count] = i;
            top_count += 1u;
        } else if (entry->state.slot == FISSION_NK_PANEL_SLOT_BOTTOM) {
            bottom_indices[bottom_count] = i;
            bottom_count += 1u;
        } else {
            center_indices[center_count] = i;
            center_count += 1u;
        }
    }

    has_left = (left_count > 0u);
    has_center = (center_count > 0u);
    has_right = (right_count > 0u);
    has_top = (top_count > 0u);
    has_bottom = (bottom_count > 0u);
    has_mid = (has_left != 0 || has_center != 0 || has_right != 0);

    horizontal_gutters = 0;
    if (has_top != 0 && (has_mid != 0 || has_bottom != 0)) {
        horizontal_gutters += 1;
    }
    if (has_mid != 0 && has_bottom != 0) {
        horizontal_gutters += 1;
    }

    shared_h = content_h - FISSION_NK_PANEL_GAP * (float)horizontal_gutters;
    if (shared_h < 1.0f) {
        shared_h = 1.0f;
    }

    top_h = 0.0f;
    mid_h = 0.0f;
    bottom_h = 0.0f;

    if (has_top != 0 && has_mid != 0 && has_bottom != 0) {
        top_h = shared_h * host->top_row_ratio;
        bottom_h = shared_h * host->bottom_row_ratio;
        mid_h = shared_h - top_h - bottom_h;
    } else if (has_top != 0 && has_mid != 0) {
        top_h = shared_h * host->top_row_ratio;
        mid_h = shared_h - top_h;
    } else if (has_mid != 0 && has_bottom != 0) {
        bottom_h = shared_h * host->bottom_row_ratio;
        mid_h = shared_h - bottom_h;
    } else if (has_top != 0 && has_bottom != 0) {
        float ratio_sum;

        ratio_sum = host->top_row_ratio + host->bottom_row_ratio;
        if (ratio_sum <= 0.001f) {
            ratio_sum = 1.0f;
        }
        top_h = shared_h * (host->top_row_ratio / ratio_sum);
        bottom_h = shared_h - top_h;
    } else if (has_top != 0) {
        top_h = shared_h;
    } else if (has_mid != 0) {
        mid_h = shared_h;
    } else if (has_bottom != 0) {
        bottom_h = shared_h;
    }

    top_min = (has_top != 0) ? FISSION_NK_PANEL_MIN_TOP_HEIGHT : 0.0f;
    mid_min = (has_mid != 0) ? FISSION_NK_PANEL_MIN_HEIGHT : 0.0f;
    bottom_min = (has_bottom != 0) ? FISSION_NK_PANEL_MIN_BOTTOM_HEIGHT : 0.0f;

    if (has_mid != 0 && mid_h < mid_min) {
        deficit = mid_min - mid_h;
        deficit = fission_nk_panel_take_extent(&top_h, top_min, deficit);
        deficit = fission_nk_panel_take_extent(&bottom_h, bottom_min, deficit);
        mid_h = mid_min - deficit;
    }
    if (has_top != 0 && top_h < top_min) {
        deficit = top_min - top_h;
        deficit = fission_nk_panel_take_extent(&mid_h, mid_min, deficit);
        deficit = fission_nk_panel_take_extent(&bottom_h, bottom_min, deficit);
        top_h = top_min - deficit;
    }
    if (has_bottom != 0 && bottom_h < bottom_min) {
        deficit = bottom_min - bottom_h;
        deficit = fission_nk_panel_take_extent(&mid_h, mid_min, deficit);
        deficit = fission_nk_panel_take_extent(&top_h, top_min, deficit);
        bottom_h = bottom_min - deficit;
    }

    if (has_top != 0 && top_h < 1.0f) {
        top_h = 1.0f;
    }
    if (has_mid != 0 && mid_h < 1.0f) {
        mid_h = 1.0f;
    }
    if (has_bottom != 0 && bottom_h < 1.0f) {
        bottom_h = 1.0f;
    }

    cursor_y = content_y;
    top_y = cursor_y;
    mid_y = cursor_y;
    bottom_y = cursor_y;

    if (has_top != 0) {
        top_y = cursor_y;
        cursor_y += top_h;
        if (has_mid != 0 || has_bottom != 0) {
            host->splitter_top_bounds.x = content_x;
            host->splitter_top_bounds.y = cursor_y;
            host->splitter_top_bounds.w = content_w;
            host->splitter_top_bounds.h = FISSION_NK_PANEL_GAP;
            cursor_y += FISSION_NK_PANEL_GAP;
        }
    }
    if (has_mid != 0) {
        mid_y = cursor_y;
        cursor_y += mid_h;
        if (has_bottom != 0) {
            host->splitter_bottom_bounds.x = content_x;
            host->splitter_bottom_bounds.y = cursor_y;
            host->splitter_bottom_bounds.w = content_w;
            host->splitter_bottom_bounds.h = FISSION_NK_PANEL_GAP;
            cursor_y += FISSION_NK_PANEL_GAP;
        }
    }
    if (has_bottom != 0) {
        bottom_y = cursor_y;
    }

    if (has_top != 0) {
        fission_nk_panel_layout_stack_horizontal(
            host,
            top_indices,
            top_count,
            content_x,
            top_y,
            content_w,
            top_h
        );
    }
    if (has_bottom != 0) {
        fission_nk_panel_layout_stack_horizontal(
            host,
            bottom_indices,
            bottom_count,
            content_x,
            bottom_y,
            content_w,
            bottom_h
        );
    }

    if (has_mid == 0) {
        return 1;
    }

    vertical_gutters = 0;
    if (has_left != 0 && (has_center != 0 || has_right != 0)) {
        vertical_gutters += 1;
    }
    if (has_center != 0 && has_right != 0) {
        vertical_gutters += 1;
    }

    shared_w = content_w - FISSION_NK_PANEL_GAP * (float)vertical_gutters;
    if (shared_w < 1.0f) {
        shared_w = 1.0f;
    }

    left_w = 0.0f;
    center_w = 0.0f;
    right_w = 0.0f;

    if (has_left != 0 && has_center != 0 && has_right != 0) {
        left_w = shared_w * host->left_column_ratio;
        right_w = shared_w * host->right_column_ratio;
        center_w = shared_w - left_w - right_w;
    } else if (has_left != 0 && has_center != 0) {
        left_w = shared_w * host->left_column_ratio;
        center_w = shared_w - left_w;
    } else if (has_center != 0 && has_right != 0) {
        right_w = shared_w * host->right_column_ratio;
        center_w = shared_w - right_w;
    } else if (has_left != 0 && has_right != 0) {
        float ratio_sum;

        ratio_sum = host->left_column_ratio + host->right_column_ratio;
        if (ratio_sum <= 0.001f) {
            ratio_sum = 1.0f;
        }
        left_w = shared_w * (host->left_column_ratio / ratio_sum);
        right_w = shared_w - left_w;
    } else if (has_left != 0) {
        left_w = shared_w;
    } else if (has_center != 0) {
        center_w = shared_w;
    } else if (has_right != 0) {
        right_w = shared_w;
    }

    left_min = (has_left != 0) ? FISSION_NK_PANEL_MIN_LEFT_WIDTH : 0.0f;
    center_min = (has_center != 0) ? FISSION_NK_PANEL_MIN_CENTER_WIDTH : 0.0f;
    right_min = (has_right != 0) ? FISSION_NK_PANEL_MIN_RIGHT_WIDTH : 0.0f;

    if (has_center != 0 && center_w < center_min) {
        deficit = center_min - center_w;
        deficit = fission_nk_panel_take_extent(&left_w, left_min, deficit);
        deficit = fission_nk_panel_take_extent(&right_w, right_min, deficit);
        center_w = center_min - deficit;
    }
    if (has_left != 0 && left_w < left_min) {
        deficit = left_min - left_w;
        deficit = fission_nk_panel_take_extent(&center_w, center_min, deficit);
        deficit = fission_nk_panel_take_extent(&right_w, right_min, deficit);
        left_w = left_min - deficit;
    }
    if (has_right != 0 && right_w < right_min) {
        deficit = right_min - right_w;
        deficit = fission_nk_panel_take_extent(&center_w, center_min, deficit);
        deficit = fission_nk_panel_take_extent(&left_w, left_min, deficit);
        right_w = right_min - deficit;
    }

    if (has_left != 0 && left_w < 1.0f) {
        left_w = 1.0f;
    }
    if (has_center != 0 && center_w < 1.0f) {
        center_w = 1.0f;
    }
    if (has_right != 0 && right_w < 1.0f) {
        right_w = 1.0f;
    }

    cursor_x = content_x;
    left_x = cursor_x;
    center_x = cursor_x;
    right_x = cursor_x;

    if (has_left != 0) {
        left_x = cursor_x;
        cursor_x += left_w;
        if (has_center != 0 || has_right != 0) {
            host->splitter_left_bounds.x = cursor_x;
            host->splitter_left_bounds.y = mid_y;
            host->splitter_left_bounds.w = FISSION_NK_PANEL_GAP;
            host->splitter_left_bounds.h = mid_h;
            cursor_x += FISSION_NK_PANEL_GAP;
        }
    }
    if (has_center != 0) {
        center_x = cursor_x;
        cursor_x += center_w;
        if (has_right != 0) {
            host->splitter_right_bounds.x = cursor_x;
            host->splitter_right_bounds.y = mid_y;
            host->splitter_right_bounds.w = FISSION_NK_PANEL_GAP;
            host->splitter_right_bounds.h = mid_h;
            cursor_x += FISSION_NK_PANEL_GAP;
        }
    }
    if (has_right != 0) {
        right_x = cursor_x;
    }

    if (has_left != 0) {
        fission_nk_panel_layout_stack_vertical(
            host,
            left_indices,
            left_count,
            left_x,
            mid_y,
            left_w,
            mid_h
        );
    }
    if (has_center != 0) {
        fission_nk_panel_layout_stack_vertical(
            host,
            center_indices,
            center_count,
            center_x,
            mid_y,
            center_w,
            mid_h
        );
    }
    if (has_right != 0) {
        fission_nk_panel_layout_stack_vertical(
            host,
            right_indices,
            right_count,
            right_x,
            mid_y,
            right_w,
            mid_h
        );
    }

    return 1;
}

static int fission_nk_panel_host_update_splitters(
    fission_nk_panel_workspace_t *host,
    struct nk_context *ctx
)
{
    int changed;
    int hovered_splitter_id;
    float delta;
    struct nk_rect splitter_rect;

    if (host == NULL || ctx == NULL) {
        return 0;
    }

    changed = 0;
    hovered_splitter_id = FISSION_NK_PANEL_SPLITTER_NONE;

    if (
        (host->active_splitter == FISSION_NK_PANEL_SPLITTER_LEFT &&
            host->splitter_left_bounds.w <= 0.0f) ||
        (host->active_splitter == FISSION_NK_PANEL_SPLITTER_RIGHT &&
            host->splitter_right_bounds.w <= 0.0f) ||
        (host->active_splitter == FISSION_NK_PANEL_SPLITTER_TOP &&
            host->splitter_top_bounds.w <= 0.0f) ||
        (host->active_splitter == FISSION_NK_PANEL_SPLITTER_BOTTOM &&
            host->splitter_bottom_bounds.w <= 0.0f)
    ) {
        host->active_splitter = FISSION_NK_PANEL_SPLITTER_NONE;
    }

    if (host->splitter_left_bounds.w > 0.0f && host->splitter_left_bounds.h > 0.0f) {
        splitter_rect = fission_nk_panel_bounds_to_nk_rect(&host->splitter_left_bounds);
        if (
            fission_nk_update_splitter_interaction(
                ctx,
                &splitter_rect,
                1,
                FISSION_NK_PANEL_SPLITTER_LEFT,
                &host->active_splitter,
                &hovered_splitter_id,
                &delta
            ) != 0 &&
            delta != 0.0f
        ) {
            fission_nk_panel_workspace_set_column_ratios(
                host,
                host->left_column_ratio + delta / host->dock_workspace_bounds.w,
                host->right_column_ratio
            );
            changed = 1;
        }
    }

    if (host->splitter_right_bounds.w > 0.0f && host->splitter_right_bounds.h > 0.0f) {
        splitter_rect = fission_nk_panel_bounds_to_nk_rect(&host->splitter_right_bounds);
        if (
            fission_nk_update_splitter_interaction(
                ctx,
                &splitter_rect,
                1,
                FISSION_NK_PANEL_SPLITTER_RIGHT,
                &host->active_splitter,
                &hovered_splitter_id,
                &delta
            ) != 0 &&
            delta != 0.0f
        ) {
            fission_nk_panel_workspace_set_column_ratios(
                host,
                host->left_column_ratio,
                host->right_column_ratio - delta / host->dock_workspace_bounds.w
            );
            changed = 1;
        }
    }

    if (host->splitter_top_bounds.w > 0.0f && host->splitter_top_bounds.h > 0.0f) {
        splitter_rect = fission_nk_panel_bounds_to_nk_rect(&host->splitter_top_bounds);
        if (
            fission_nk_update_splitter_interaction(
                ctx,
                &splitter_rect,
                0,
                FISSION_NK_PANEL_SPLITTER_TOP,
                &host->active_splitter,
                &hovered_splitter_id,
                &delta
            ) != 0 &&
            delta != 0.0f
        ) {
            fission_nk_panel_workspace_set_row_ratios(
                host,
                host->top_row_ratio + delta / host->dock_workspace_bounds.h,
                host->bottom_row_ratio
            );
            changed = 1;
        }
    }

    if (host->splitter_bottom_bounds.w > 0.0f && host->splitter_bottom_bounds.h > 0.0f) {
        splitter_rect = fission_nk_panel_bounds_to_nk_rect(&host->splitter_bottom_bounds);
        if (
            fission_nk_update_splitter_interaction(
                ctx,
                &splitter_rect,
                0,
                FISSION_NK_PANEL_SPLITTER_BOTTOM,
                &host->active_splitter,
                &hovered_splitter_id,
                &delta
            ) != 0 &&
            delta != 0.0f
        ) {
            fission_nk_panel_workspace_set_row_ratios(
                host,
                host->top_row_ratio,
                host->bottom_row_ratio - delta / host->dock_workspace_bounds.h
            );
            changed = 1;
        }
    }

    host->hovered_splitter = hovered_splitter_id;
    if (host->active_splitter != FISSION_NK_PANEL_SPLITTER_NONE) {
        host->hovered_splitter = host->active_splitter;
    }

    return changed;
}

static void fission_nk_panel_host_draw_splitter_overlays(
    fission_nk_panel_workspace_t *host,
    struct nk_context *ctx
)
{
    struct nk_rect splitter_rect;
    char overlay_id[96];

    if (host == NULL || ctx == NULL) {
        return;
    }

    if (host->splitter_left_bounds.w > 0.0f && host->splitter_left_bounds.h > 0.0f) {
        fission_nk_panel_overlay_id(overlay_id, sizeof(overlay_id), host, "splitter_left");
        splitter_rect = fission_nk_panel_bounds_to_nk_rect(&host->splitter_left_bounds);
        fission_nk_draw_splitter_overlay(
            ctx,
            overlay_id,
            &splitter_rect,
            1,
            host->active_splitter == FISSION_NK_PANEL_SPLITTER_LEFT,
            host->hovered_splitter == FISSION_NK_PANEL_SPLITTER_LEFT
        );
    }

    if (host->splitter_right_bounds.w > 0.0f && host->splitter_right_bounds.h > 0.0f) {
        fission_nk_panel_overlay_id(overlay_id, sizeof(overlay_id), host, "splitter_right");
        splitter_rect = fission_nk_panel_bounds_to_nk_rect(&host->splitter_right_bounds);
        fission_nk_draw_splitter_overlay(
            ctx,
            overlay_id,
            &splitter_rect,
            1,
            host->active_splitter == FISSION_NK_PANEL_SPLITTER_RIGHT,
            host->hovered_splitter == FISSION_NK_PANEL_SPLITTER_RIGHT
        );
    }

    if (host->splitter_top_bounds.w > 0.0f && host->splitter_top_bounds.h > 0.0f) {
        fission_nk_panel_overlay_id(overlay_id, sizeof(overlay_id), host, "splitter_top");
        splitter_rect = fission_nk_panel_bounds_to_nk_rect(&host->splitter_top_bounds);
        fission_nk_draw_splitter_overlay(
            ctx,
            overlay_id,
            &splitter_rect,
            0,
            host->active_splitter == FISSION_NK_PANEL_SPLITTER_TOP,
            host->hovered_splitter == FISSION_NK_PANEL_SPLITTER_TOP
        );
    }

    if (host->splitter_bottom_bounds.w > 0.0f && host->splitter_bottom_bounds.h > 0.0f) {
        fission_nk_panel_overlay_id(overlay_id, sizeof(overlay_id), host, "splitter_bottom");
        splitter_rect = fission_nk_panel_bounds_to_nk_rect(&host->splitter_bottom_bounds);
        fission_nk_draw_splitter_overlay(
            ctx,
            overlay_id,
            &splitter_rect,
            0,
            host->active_splitter == FISSION_NK_PANEL_SPLITTER_BOTTOM,
            host->hovered_splitter == FISSION_NK_PANEL_SPLITTER_BOTTOM
        );
    }
}

static int fission_nk_panel_host_mouse_over_detached_panel(
    const fission_nk_panel_workspace_t *host,
    float mouse_x,
    float mouse_y
);

static void fission_nk_panel_host_begin_panel_drag(
    fission_nk_panel_workspace_t *host,
    struct nk_context *ctx
)
{
    float mouse_x;
    float mouse_y;
    size_t i;

    if (host == NULL || ctx == NULL || host->dragging_panel != 0) {
        return;
    }
    if (host->active_splitter != FISSION_NK_PANEL_SPLITTER_NONE) {
        return;
    }
    if (!nk_input_is_mouse_pressed(&ctx->input, NK_BUTTON_LEFT)) {
        return;
    }

    mouse_x = ctx->input.mouse.pos.x;
    mouse_y = ctx->input.mouse.pos.y;
    if (fission_nk_panel_host_mouse_over_detached_panel(host, mouse_x, mouse_y) != 0) {
        return;
    }

    for (i = host->count; i > 0u; --i) {
        fission_nk_panel_entry_t *entry;
        fission_nk_panel_bounds_t title_bounds;

        entry = &host->entries[i - 1u];
        if (entry->state.visible == 0 || entry->state.detached != 0) {
            continue;
        }

        title_bounds = entry->state.resolved_bounds;
        title_bounds.h = FISSION_NK_PANEL_TITLE_BAR_HEIGHT;
        if (title_bounds.h > entry->state.resolved_bounds.h) {
            title_bounds.h = entry->state.resolved_bounds.h;
        }

        if (fission_nk_panel_point_in_bounds(&title_bounds, mouse_x, mouse_y) == 0) {
            continue;
        }

        if (entry->state.detachable != 0) {
            fission_nk_panel_bounds_t button_bounds;

            button_bounds = fission_nk_panel_header_button_bounds(&entry->state.resolved_bounds);
            if (fission_nk_panel_point_in_bounds(&button_bounds, mouse_x, mouse_y) != 0) {
                continue;
            }
        }

        host->dragging_panel = 1;
        host->dragging_panel_index = i - 1u;
        host->drag_target_slot = entry->state.slot;
        host->dragging_has_moved = 0;
        host->dragging_start_x = mouse_x;
        host->dragging_start_y = mouse_y;
        break;
    }
}

static int fission_nk_panel_host_mouse_over_detached_panel(
    const fission_nk_panel_workspace_t *host,
    float mouse_x,
    float mouse_y
)
{
    size_t i;

    if (host == NULL) {
        return 0;
    }

    for (i = host->count; i > 0u; --i) {
        const fission_nk_panel_entry_t *entry;

        entry = &host->entries[i - 1u];
        if (entry->state.visible == 0 || entry->state.detached == 0) {
            continue;
        }
        if (
            fission_nk_panel_point_in_bounds(
                &entry->state.resolved_bounds,
                mouse_x,
                mouse_y
            ) == 0
        ) {
            continue;
        }

        return 1;
    }

    return 0;
}

static int fission_nk_panel_host_update_panel_drag(
    fission_nk_panel_workspace_t *host,
    struct nk_context *ctx
)
{
    struct nk_rect dock_bounds;
    struct nk_rect zones[FISSION_NK_DOCK_ZONE_COUNT];
    fission_nk_dock_zone_t zone;
    float mouse_x;
    float mouse_y;

    if (host == NULL || ctx == NULL || host->dragging_panel == 0) {
        return 0;
    }
    if (host->dragging_panel_index >= host->count) {
        host->dragging_panel = 0;
        host->dragging_has_moved = 0;
        return 0;
    }

    dock_bounds = fission_nk_panel_bounds_to_nk_rect(&host->dock_workspace_bounds);
    fission_nk_build_dock_zones(
        &dock_bounds,
        FISSION_NK_PANEL_DOCK_EDGE_FRACTION,
        FISSION_NK_PANEL_DOCK_MIN_EDGE_SIZE,
        zones
    );

    mouse_x = ctx->input.mouse.pos.x;
    mouse_y = ctx->input.mouse.pos.y;

    if (host->dragging_has_moved == 0) {
        float dx;
        float dy;

        dx = mouse_x - host->dragging_start_x;
        dy = mouse_y - host->dragging_start_y;
        if (dx * dx + dy * dy >= 25.0f) {
            host->dragging_has_moved = 1;
        }
    }

    if (
        host->dragging_has_moved == 0 &&
        nk_input_is_mouse_down(&ctx->input, NK_BUTTON_LEFT)
    ) {
        return 0;
    }

    if (
        host->dragging_has_moved == 0 &&
        !nk_input_is_mouse_down(&ctx->input, NK_BUTTON_LEFT)
    ) {
        host->dragging_panel = 0;
        return 0;
    }

    zone = fission_nk_pick_dock_zone(zones, mouse_x, mouse_y);
    if (zone != FISSION_NK_DOCK_ZONE_NONE) {
        host->drag_target_slot = fission_nk_panel_slot_from_dock_zone(zone);
    }

    if (nk_input_is_mouse_down(&ctx->input, NK_BUTTON_LEFT)) {
        return 0;
    }

    if (zone != FISSION_NK_DOCK_ZONE_NONE) {
        host->entries[host->dragging_panel_index].state.slot = fission_nk_panel_slot_from_dock_zone(zone);
        host->entries[host->dragging_panel_index].state.detached = 0;

        host->dragging_panel = 0;
        host->dragging_has_moved = 0;
        return 1;
    }

    host->dragging_panel = 0;
    host->dragging_has_moved = 0;
    return 0;
}

static void fission_nk_panel_host_draw_drag_overlay(
    const fission_nk_panel_workspace_t *host,
    struct nk_context *ctx
)
{
    struct nk_rect dock_bounds;
    struct nk_rect zones[FISSION_NK_DOCK_ZONE_COUNT];
    fission_nk_dock_zone_t zone;
    char overlay_id[96];

    if (
        host == NULL ||
        ctx == NULL ||
        host->dragging_panel == 0 ||
        host->dragging_has_moved == 0
    ) {
        return;
    }

    dock_bounds = fission_nk_panel_bounds_to_nk_rect(&host->dock_workspace_bounds);
    fission_nk_build_dock_zones(
        &dock_bounds,
        FISSION_NK_PANEL_DOCK_EDGE_FRACTION,
        FISSION_NK_PANEL_DOCK_MIN_EDGE_SIZE,
        zones
    );
    zone = fission_nk_pick_dock_zone(
        zones,
        ctx->input.mouse.pos.x,
        ctx->input.mouse.pos.y
    );
    if (zone == FISSION_NK_DOCK_ZONE_NONE) {
        zone = fission_nk_panel_dock_zone_from_slot(host->drag_target_slot);
    }

    fission_nk_panel_overlay_id(overlay_id, sizeof(overlay_id), host, "dock_overlay");
    fission_nk_draw_dock_zones_overlay(
        ctx,
        overlay_id,
        &dock_bounds,
        zones,
        zone
    );
}

void fission_nk_panel_workspace_init(
    fission_nk_panel_workspace_t *host
)
{
    if (host == NULL) {
        return;
    }

    memset(host, 0, sizeof(*host));

    host->left_column_ratio = 0.24f;
    host->right_column_ratio = 0.23f;
    host->top_row_ratio = 0.22f;
    host->bottom_row_ratio = 0.20f;
    host->last_window_width = 1600;
    host->last_window_height = 900;
    host->active_splitter = FISSION_NK_PANEL_SPLITTER_NONE;
    host->hovered_splitter = FISSION_NK_PANEL_SPLITTER_NONE;
    host->dragging_panel = 0;
    host->dragging_panel_index = 0u;
    host->drag_target_slot = FISSION_NK_PANEL_SLOT_CENTER;
    host->dragging_has_moved = 0;
    host->dragging_start_x = 0.0f;
    host->dragging_start_y = 0.0f;
    fission_nk_panel_bounds_zero(&host->dock_workspace_bounds);
    fission_nk_panel_bounds_zero(&host->splitter_left_bounds);
    fission_nk_panel_bounds_zero(&host->splitter_right_bounds);
    fission_nk_panel_bounds_zero(&host->splitter_top_bounds);
    fission_nk_panel_bounds_zero(&host->splitter_bottom_bounds);
}

fission_nk_panel_status_t fission_nk_panel_workspace_register(
    fission_nk_panel_workspace_t *host,
    const fission_nk_panel_desc_t *panel
)
{
    size_t i;
    fission_nk_panel_entry_t *entry;

    if (host == NULL || panel == NULL) {
        return FISSION_NK_PANEL_STATUS_INVALID_ARGUMENT;
    }
    if (panel->id == NULL || panel->title == NULL || panel->draw == NULL) {
        return FISSION_NK_PANEL_STATUS_INVALID_ARGUMENT;
    }
    if (host->count >= FISSION_NK_MAX_PANELS) {
        return FISSION_NK_PANEL_STATUS_RUNTIME_ERROR;
    }

    for (i = 0u; i < host->count; ++i) {
        if (strcmp(host->entries[i].desc.id, panel->id) == 0) {
            return FISSION_NK_PANEL_STATUS_INVALID_ARGUMENT;
        }
    }

    if (panel->init != NULL) {
        fission_nk_panel_status_t init_status;

        init_status = panel->init(panel->user_data);
        if (init_status != FISSION_NK_PANEL_STATUS_OK) {
            return init_status;
        }
    }

    entry = &host->entries[host->count];
    memset(entry, 0, sizeof(*entry));
    entry->desc = *panel;

    entry->state.visible = (panel->default_visible >= 0) ? 1 : 0;
    entry->state.detachable = (panel->default_detachable >= 0) ? 1 : 0;
    entry->state.detached = 0;

    if (
        panel->default_slot == FISSION_NK_PANEL_SLOT_LEFT ||
        panel->default_slot == FISSION_NK_PANEL_SLOT_RIGHT ||
        panel->default_slot == FISSION_NK_PANEL_SLOT_TOP ||
        panel->default_slot == FISSION_NK_PANEL_SLOT_BOTTOM
    ) {
        entry->state.slot = panel->default_slot;
    } else {
        entry->state.slot = FISSION_NK_PANEL_SLOT_CENTER;
    }

    if (
        panel->default_detached_bounds.w > 0.0f &&
        panel->default_detached_bounds.h > 0.0f
    ) {
        entry->state.detached_bounds = panel->default_detached_bounds;
    } else {
        entry->state.detached_bounds = fission_nk_panel_default_detached_bounds(entry->state.slot);
    }

    fission_nk_panel_sanitize_detached_bounds(host, &entry->state.detached_bounds);
    entry->state.resolved_bounds = entry->state.detached_bounds;

    host->count += 1u;
    return FISSION_NK_PANEL_STATUS_OK;
}

void fission_nk_panel_workspace_draw_all(
    fission_nk_panel_workspace_t *host,
    struct nk_context *ctx,
    int window_width,
    int window_height
)
{
    size_t i;
    int layout_changed;
    int visible_snapshot[FISSION_NK_MAX_PANELS];
    int detached_snapshot[FISSION_NK_MAX_PANELS];
    int drawn[FISSION_NK_MAX_PANELS];

    if (host == NULL || ctx == NULL || window_width <= 0 || window_height <= 0) {
        return;
    }

    (void)fission_nk_panel_host_resolve_layout(host, window_width, window_height);

    layout_changed = fission_nk_panel_host_update_splitters(host, ctx);
    if (layout_changed != 0) {
        (void)fission_nk_panel_host_resolve_layout(host, window_width, window_height);
    }

    fission_nk_panel_host_begin_panel_drag(host, ctx);
    if (fission_nk_panel_host_update_panel_drag(host, ctx) != 0) {
        (void)fission_nk_panel_host_resolve_layout(host, window_width, window_height);
    }

    for (i = 0u; i < host->count; ++i) {
        visible_snapshot[i] = host->entries[i].state.visible;
        detached_snapshot[i] = host->entries[i].state.detached;
        drawn[i] = 0;
    }

    for (i = 0u; i < host->count; ++i) {
        fission_nk_panel_entry_t *entry;

        entry = &host->entries[i];
        if (visible_snapshot[i] == 0 || detached_snapshot[i] != 0) {
            continue;
        }

        entry->desc.draw(
            ctx,
            host,
            entry->desc.id,
            window_width,
            window_height,
            entry->desc.user_data
        );
        drawn[i] = 1;
    }

    for (i = 0u; i < host->count; ++i) {
        fission_nk_panel_entry_t *entry;

        entry = &host->entries[i];
        if (drawn[i] != 0 || visible_snapshot[i] == 0 || detached_snapshot[i] == 0) {
            continue;
        }

        entry->desc.draw(
            ctx,
            host,
            entry->desc.id,
            window_width,
            window_height,
            entry->desc.user_data
        );
        drawn[i] = 1;
    }

    fission_nk_panel_host_draw_splitter_overlays(host, ctx);
    fission_nk_panel_host_draw_drag_overlay(host, ctx);
}

void fission_nk_panel_workspace_shutdown(fission_nk_panel_workspace_t *host)
{
    size_t i;

    if (host == NULL) {
        return;
    }

    for (i = host->count; i > 0u; --i) {
        fission_nk_panel_desc_t *desc;

        desc = &host->entries[i - 1u].desc;
        if (desc->shutdown != NULL) {
            desc->shutdown(desc->user_data);
        }
    }

    host->count = 0u;
}

size_t fission_nk_panel_workspace_count(const fission_nk_panel_workspace_t *host)
{
    if (host == NULL) {
        return 0u;
    }
    return host->count;
}

const char *fission_nk_panel_workspace_panel_id_at(
    const fission_nk_panel_workspace_t *host,
    size_t index
)
{
    if (host == NULL || index >= host->count) {
        return NULL;
    }
    return host->entries[index].desc.id;
}

const char *fission_nk_panel_workspace_panel_title_at(
    const fission_nk_panel_workspace_t *host,
    size_t index
)
{
    if (host == NULL || index >= host->count) {
        return NULL;
    }
    return host->entries[index].desc.title;
}

int fission_nk_panel_workspace_panel_is_visible_at(
    const fission_nk_panel_workspace_t *host,
    size_t index
)
{
    if (host == NULL || index >= host->count) {
        return 0;
    }
    return host->entries[index].state.visible;
}

int fission_nk_panel_workspace_panel_is_detached_at(
    const fission_nk_panel_workspace_t *host,
    size_t index
)
{
    if (host == NULL || index >= host->count) {
        return 0;
    }
    return host->entries[index].state.detached;
}

int fission_nk_panel_workspace_panel_is_detachable_at(
    const fission_nk_panel_workspace_t *host,
    size_t index
)
{
    if (host == NULL || index >= host->count) {
        return 0;
    }
    return host->entries[index].state.detachable;
}

fission_nk_panel_slot_t fission_nk_panel_workspace_panel_slot_at(
    const fission_nk_panel_workspace_t *host,
    size_t index
)
{
    if (host == NULL || index >= host->count) {
        return FISSION_NK_PANEL_SLOT_CENTER;
    }
    return host->entries[index].state.slot;
}

fission_nk_panel_status_t fission_nk_panel_workspace_set_panel_visible_at(
    fission_nk_panel_workspace_t *host,
    size_t index,
    int visible
)
{
    if (host == NULL || index >= host->count) {
        return FISSION_NK_PANEL_STATUS_INVALID_ARGUMENT;
    }

    host->entries[index].state.visible = (visible != 0) ? 1 : 0;
    if (host->entries[index].state.visible == 0) {
        host->entries[index].state.detached = 0;
        if (host->dragging_panel != 0 && host->dragging_panel_index == index) {
            host->dragging_panel = 0;
            host->dragging_has_moved = 0;
        }
    }
    return FISSION_NK_PANEL_STATUS_OK;
}

fission_nk_panel_status_t fission_nk_panel_workspace_set_panel_detached_at(
    fission_nk_panel_workspace_t *host,
    size_t index,
    int detached
)
{
    if (host == NULL || index >= host->count) {
        return FISSION_NK_PANEL_STATUS_INVALID_ARGUMENT;
    }

    if (detached != 0 && host->entries[index].state.detachable == 0) {
        return FISSION_NK_PANEL_STATUS_INVALID_ARGUMENT;
    }

    host->entries[index].state.detached = (detached != 0) ? 1 : 0;
    if (host->entries[index].state.detached != 0) {
        fission_nk_panel_sanitize_detached_bounds(host, &host->entries[index].state.detached_bounds);
    }
    if (host->dragging_panel != 0 && host->dragging_panel_index == index) {
        host->dragging_panel = 0;
        host->dragging_has_moved = 0;
    }
    return FISSION_NK_PANEL_STATUS_OK;
}

fission_nk_panel_status_t fission_nk_panel_workspace_set_panel_slot_at(
    fission_nk_panel_workspace_t *host,
    size_t index,
    fission_nk_panel_slot_t slot
)
{
    if (host == NULL || index >= host->count) {
        return FISSION_NK_PANEL_STATUS_INVALID_ARGUMENT;
    }

    if (
        slot != FISSION_NK_PANEL_SLOT_LEFT &&
        slot != FISSION_NK_PANEL_SLOT_CENTER &&
        slot != FISSION_NK_PANEL_SLOT_RIGHT &&
        slot != FISSION_NK_PANEL_SLOT_TOP &&
        slot != FISSION_NK_PANEL_SLOT_BOTTOM
    ) {
        return FISSION_NK_PANEL_STATUS_INVALID_ARGUMENT;
    }

    host->entries[index].state.slot = slot;
    return FISSION_NK_PANEL_STATUS_OK;
}

fission_nk_panel_status_t fission_nk_panel_workspace_get_panel_bounds(
    const fission_nk_panel_workspace_t *host,
    const char *panel_id,
    fission_nk_panel_bounds_t *out_bounds
)
{
    size_t index;

    if (host == NULL || panel_id == NULL || out_bounds == NULL) {
        return FISSION_NK_PANEL_STATUS_INVALID_ARGUMENT;
    }

    index = fission_nk_panel_find_index(host, panel_id);
    if (index >= host->count) {
        return FISSION_NK_PANEL_STATUS_INVALID_ARGUMENT;
    }

    *out_bounds = host->entries[index].state.resolved_bounds;
    return FISSION_NK_PANEL_STATUS_OK;
}

int fission_nk_panel_workspace_panel_is_detached(
    const fission_nk_panel_workspace_t *host,
    const char *panel_id
)
{
    size_t index;

    if (host == NULL || panel_id == NULL) {
        return 0;
    }

    index = fission_nk_panel_find_index(host, panel_id);
    if (index >= host->count) {
        return 0;
    }

    return host->entries[index].state.detached;
}

int fission_nk_panel_workspace_panel_is_detachable(
    const fission_nk_panel_workspace_t *host,
    const char *panel_id
)
{
    size_t index;

    if (host == NULL || panel_id == NULL) {
        return 0;
    }

    index = fission_nk_panel_find_index(host, panel_id);
    if (index >= host->count) {
        return 0;
    }

    return host->entries[index].state.detachable;
}

int fission_nk_panel_workspace_panel_is_visible(
    const fission_nk_panel_workspace_t *host,
    const char *panel_id
)
{
    size_t index;

    if (host == NULL || panel_id == NULL) {
        return 0;
    }

    index = fission_nk_panel_find_index(host, panel_id);
    if (index >= host->count) {
        return 0;
    }

    return host->entries[index].state.visible;
}

fission_nk_panel_status_t fission_nk_panel_workspace_set_panel_detached(
    fission_nk_panel_workspace_t *host,
    const char *panel_id,
    int detached
)
{
    size_t index;

    if (host == NULL || panel_id == NULL) {
        return FISSION_NK_PANEL_STATUS_INVALID_ARGUMENT;
    }

    index = fission_nk_panel_find_index(host, panel_id);
    if (index >= host->count) {
        return FISSION_NK_PANEL_STATUS_INVALID_ARGUMENT;
    }

    return fission_nk_panel_workspace_set_panel_detached_at(host, index, detached);
}

void fission_nk_panel_workspace_set_panel_detached_bounds(
    fission_nk_panel_workspace_t *host,
    const char *panel_id,
    const fission_nk_panel_bounds_t *bounds
)
{
    size_t index;

    if (host == NULL || panel_id == NULL || bounds == NULL) {
        return;
    }

    index = fission_nk_panel_find_index(host, panel_id);
    if (index >= host->count) {
        return;
    }

    host->entries[index].state.detached_bounds = *bounds;
    fission_nk_panel_sanitize_detached_bounds(host, &host->entries[index].state.detached_bounds);
}

void fission_nk_panel_workspace_get_column_ratios(
    const fission_nk_panel_workspace_t *host,
    float *out_left_ratio,
    float *out_right_ratio
)
{
    if (host == NULL) {
        return;
    }

    if (out_left_ratio != NULL) {
        *out_left_ratio = host->left_column_ratio;
    }
    if (out_right_ratio != NULL) {
        *out_right_ratio = host->right_column_ratio;
    }
}

void fission_nk_panel_workspace_set_column_ratios(
    fission_nk_panel_workspace_t *host,
    float left_ratio,
    float right_ratio
)
{
    if (host == NULL) {
        return;
    }

    left_ratio = fission_nk_panel_clamp_float(left_ratio, 0.10f, 0.50f);
    right_ratio = fission_nk_panel_clamp_float(right_ratio, 0.10f, 0.50f);

    if (left_ratio + right_ratio > 0.78f) {
        float scale;

        scale = 0.78f / (left_ratio + right_ratio);
        left_ratio *= scale;
        right_ratio *= scale;
    }

    host->left_column_ratio = left_ratio;
    host->right_column_ratio = right_ratio;
}

void fission_nk_panel_workspace_get_row_ratios(
    const fission_nk_panel_workspace_t *host,
    float *out_top_ratio,
    float *out_bottom_ratio
)
{
    if (host == NULL) {
        return;
    }

    if (out_top_ratio != NULL) {
        *out_top_ratio = host->top_row_ratio;
    }
    if (out_bottom_ratio != NULL) {
        *out_bottom_ratio = host->bottom_row_ratio;
    }
}

void fission_nk_panel_workspace_set_row_ratios(
    fission_nk_panel_workspace_t *host,
    float top_ratio,
    float bottom_ratio
)
{
    if (host == NULL) {
        return;
    }

    top_ratio = fission_nk_panel_clamp_float(top_ratio, 0.10f, 0.45f);
    bottom_ratio = fission_nk_panel_clamp_float(bottom_ratio, 0.10f, 0.45f);

    if (top_ratio + bottom_ratio > 0.78f) {
        float scale;

        scale = 0.78f / (top_ratio + bottom_ratio);
        top_ratio *= scale;
        bottom_ratio *= scale;
    }

    host->top_row_ratio = top_ratio;
    host->bottom_row_ratio = bottom_ratio;
}
