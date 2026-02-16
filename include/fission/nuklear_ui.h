#ifndef FISSION_NUKLEAR_UI_H
#define FISSION_NUKLEAR_UI_H

#include <stddef.h>

struct nk_context;
struct nk_rect;

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

void fission_nk_draw_splitter_overlay(
    struct nk_context *ctx,
    const char *name,
    const struct nk_rect *rect,
    int vertical,
    int active,
    int hovered
);

#endif
