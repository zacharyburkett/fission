#ifndef FISSION_NUKLEAR_UI_H
#define FISSION_NUKLEAR_UI_H

#include <stddef.h>

struct nk_context;
struct nk_rect;

#define FISSION_NK_DOCK_ZONE_COUNT 9

typedef enum fission_nk_dock_zone {
    FISSION_NK_DOCK_ZONE_NONE = -1,
    FISSION_NK_DOCK_ZONE_LEFT = 0,
    FISSION_NK_DOCK_ZONE_RIGHT = 1,
    FISSION_NK_DOCK_ZONE_TOP = 2,
    FISSION_NK_DOCK_ZONE_BOTTOM = 3,
    FISSION_NK_DOCK_ZONE_CENTER = 4,
    FISSION_NK_DOCK_ZONE_TOP_LEFT = 5,
    FISSION_NK_DOCK_ZONE_TOP_RIGHT = 6,
    FISSION_NK_DOCK_ZONE_BOTTOM_LEFT = 7,
    FISSION_NK_DOCK_ZONE_BOTTOM_RIGHT = 8
} fission_nk_dock_zone_t;

void fission_nk_apply_theme(struct nk_context *ctx);

void fission_nk_make_window_id(
    char *buffer,
    size_t buffer_size,
    const char *window_id_prefix,
    const char *window_name
);

void fission_nk_rect_translate(struct nk_rect *rect, float offset_x, float offset_y);
void fission_nk_rect_intersection(
    const struct nk_rect *a,
    const struct nk_rect *b,
    struct nk_rect *out_rect
);
void fission_nk_focus_current_window_on_scroll(struct nk_context *ctx);

void fission_nk_draw_splitter_overlay(
    struct nk_context *ctx,
    const char *name,
    const struct nk_rect *rect,
    int vertical,
    int active,
    int hovered
);

int fission_nk_begin_titled_group_in_space(
    struct nk_context *ctx,
    const char *group_id,
    const char *title,
    const struct nk_rect *bounds,
    unsigned int flags
);

void fission_nk_draw_splitter_canvas(
    struct nk_context *ctx,
    const struct nk_rect *rect,
    int vertical,
    int active,
    int hovered
);

int fission_nk_update_splitter_interaction(
    struct nk_context *ctx,
    const struct nk_rect *rect,
    int vertical,
    int splitter_id,
    int *active_splitter_id,
    int *hovered_splitter_id,
    float *out_delta
);

void fission_nk_build_dock_zones(
    const struct nk_rect *bounds,
    float edge_fraction,
    float min_edge_size,
    struct nk_rect *out_zones
);

fission_nk_dock_zone_t fission_nk_pick_dock_zone(
    const struct nk_rect *zones,
    float x,
    float y
);

void fission_nk_draw_dock_zones_overlay(
    struct nk_context *ctx,
    const char *name,
    const struct nk_rect *bounds,
    const struct nk_rect *zones,
    fission_nk_dock_zone_t hovered_zone
);

#endif
