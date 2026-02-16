#ifndef FISSION_NUKLEAR_PANELS_H
#define FISSION_NUKLEAR_PANELS_H

#include <stddef.h>

struct nk_context;

#define FISSION_NK_MAX_PANELS 32
#define FISSION_NK_PANEL_TITLE_BAR_HEIGHT 28.0f
#define FISSION_NK_PANEL_HEADER_BUTTON_WIDTH 56.0f
#define FISSION_NK_PANEL_HEADER_BUTTON_HEIGHT 18.0f
#define FISSION_NK_PANEL_HEADER_BUTTON_MARGIN 6.0f
#define FISSION_NK_PANEL_WINDOW_NO_SCROLL_FOCUS (1u << 30)

typedef struct fission_nk_panel_workspace fission_nk_panel_workspace_t;

typedef enum fission_nk_panel_status {
    FISSION_NK_PANEL_STATUS_OK = 0,
    FISSION_NK_PANEL_STATUS_INVALID_ARGUMENT = 1,
    FISSION_NK_PANEL_STATUS_RUNTIME_ERROR = 2
} fission_nk_panel_status_t;

typedef enum fission_nk_panel_slot {
    FISSION_NK_PANEL_SLOT_LEFT = 0,
    FISSION_NK_PANEL_SLOT_CENTER = 1,
    FISSION_NK_PANEL_SLOT_RIGHT = 2,
    FISSION_NK_PANEL_SLOT_TOP = 3,
    FISSION_NK_PANEL_SLOT_BOTTOM = 4
} fission_nk_panel_slot_t;

typedef struct fission_nk_panel_bounds {
    float x;
    float y;
    float w;
    float h;
} fission_nk_panel_bounds_t;

typedef void (*fission_nk_panel_workspace_reset_layout_fn)(
    fission_nk_panel_workspace_t *workspace,
    void *user_data
);

typedef fission_nk_panel_status_t (*fission_nk_panel_init_fn)(void *user_data);
typedef void (*fission_nk_panel_shutdown_fn)(void *user_data);

typedef void (*fission_nk_panel_draw_fn)(
    struct nk_context *ctx,
    fission_nk_panel_workspace_t *workspace,
    const char *panel_id,
    int window_width,
    int window_height,
    void *user_data
);

typedef struct fission_nk_panel_desc {
    const char *id;
    const char *title;
    fission_nk_panel_init_fn init;
    fission_nk_panel_shutdown_fn shutdown;
    fission_nk_panel_draw_fn draw;
    void *user_data;
    fission_nk_panel_slot_t default_slot;
    int default_visible;
    int default_detachable;
    fission_nk_panel_bounds_t default_detached_bounds;
} fission_nk_panel_desc_t;

typedef struct fission_nk_panel_state {
    int visible;
    int detached;
    int detachable;
    fission_nk_panel_slot_t slot;
    fission_nk_panel_bounds_t detached_bounds;
    fission_nk_panel_bounds_t resolved_bounds;
} fission_nk_panel_state_t;

typedef struct fission_nk_panel_entry {
    fission_nk_panel_desc_t desc;
    fission_nk_panel_state_t state;
} fission_nk_panel_entry_t;

struct fission_nk_panel_workspace {
    fission_nk_panel_entry_t entries[FISSION_NK_MAX_PANELS];
    size_t count;
    float left_column_ratio;
    float right_column_ratio;
    float top_row_ratio;
    float bottom_row_ratio;
    int last_window_width;
    int last_window_height;
    int active_splitter;
    int hovered_splitter;
    int dragging_panel;
    size_t dragging_panel_index;
    fission_nk_panel_slot_t drag_target_slot;
    int dragging_has_moved;
    float dragging_start_x;
    float dragging_start_y;
    fission_nk_panel_bounds_t dock_workspace_bounds;
    fission_nk_panel_bounds_t splitter_left_bounds;
    fission_nk_panel_bounds_t splitter_right_bounds;
    fission_nk_panel_bounds_t splitter_top_bounds;
    fission_nk_panel_bounds_t splitter_bottom_bounds;
};

void fission_nk_panel_workspace_init(fission_nk_panel_workspace_t *workspace);

fission_nk_panel_status_t fission_nk_panel_workspace_register(
    fission_nk_panel_workspace_t *workspace,
    const fission_nk_panel_desc_t *panel
);

void fission_nk_panel_workspace_draw_all(
    fission_nk_panel_workspace_t *workspace,
    struct nk_context *ctx,
    int window_width,
    int window_height
);

int fission_nk_panel_workspace_begin_window(
    struct nk_context *ctx,
    fission_nk_panel_workspace_t *workspace,
    const char *panel_id,
    const char *title,
    unsigned int extra_flags,
    fission_nk_panel_bounds_t *out_bounds
);

void fission_nk_panel_workspace_shutdown(fission_nk_panel_workspace_t *workspace);

size_t fission_nk_panel_workspace_count(const fission_nk_panel_workspace_t *workspace);
const char *fission_nk_panel_workspace_panel_id_at(
    const fission_nk_panel_workspace_t *workspace,
    size_t index
);
const char *fission_nk_panel_workspace_panel_title_at(
    const fission_nk_panel_workspace_t *workspace,
    size_t index
);
int fission_nk_panel_workspace_panel_is_visible_at(
    const fission_nk_panel_workspace_t *workspace,
    size_t index
);
int fission_nk_panel_workspace_panel_is_detached_at(
    const fission_nk_panel_workspace_t *workspace,
    size_t index
);
int fission_nk_panel_workspace_panel_is_detachable_at(
    const fission_nk_panel_workspace_t *workspace,
    size_t index
);
fission_nk_panel_slot_t fission_nk_panel_workspace_panel_slot_at(
    const fission_nk_panel_workspace_t *workspace,
    size_t index
);

fission_nk_panel_status_t fission_nk_panel_workspace_set_panel_visible_at(
    fission_nk_panel_workspace_t *workspace,
    size_t index,
    int visible
);
fission_nk_panel_status_t fission_nk_panel_workspace_set_panel_detached_at(
    fission_nk_panel_workspace_t *workspace,
    size_t index,
    int detached
);
fission_nk_panel_status_t fission_nk_panel_workspace_set_panel_slot_at(
    fission_nk_panel_workspace_t *workspace,
    size_t index,
    fission_nk_panel_slot_t slot
);

fission_nk_panel_status_t fission_nk_panel_workspace_get_panel_bounds(
    const fission_nk_panel_workspace_t *workspace,
    const char *panel_id,
    fission_nk_panel_bounds_t *out_bounds
);
int fission_nk_panel_workspace_panel_is_detached(
    const fission_nk_panel_workspace_t *workspace,
    const char *panel_id
);
int fission_nk_panel_workspace_panel_is_detachable(
    const fission_nk_panel_workspace_t *workspace,
    const char *panel_id
);
int fission_nk_panel_workspace_panel_is_visible(
    const fission_nk_panel_workspace_t *workspace,
    const char *panel_id
);
fission_nk_panel_status_t fission_nk_panel_workspace_set_panel_detached(
    fission_nk_panel_workspace_t *workspace,
    const char *panel_id,
    int detached
);
void fission_nk_panel_workspace_set_panel_detached_bounds(
    fission_nk_panel_workspace_t *workspace,
    const char *panel_id,
    const fission_nk_panel_bounds_t *bounds
);

void fission_nk_panel_workspace_get_column_ratios(
    const fission_nk_panel_workspace_t *workspace,
    float *out_left_ratio,
    float *out_right_ratio
);
void fission_nk_panel_workspace_set_column_ratios(
    fission_nk_panel_workspace_t *workspace,
    float left_ratio,
    float right_ratio
);
void fission_nk_panel_workspace_get_row_ratios(
    const fission_nk_panel_workspace_t *workspace,
    float *out_top_ratio,
    float *out_bottom_ratio
);
void fission_nk_panel_workspace_set_row_ratios(
    fission_nk_panel_workspace_t *workspace,
    float top_ratio,
    float bottom_ratio
);

void fission_nk_panel_workspace_show_all(fission_nk_panel_workspace_t *workspace);
void fission_nk_panel_workspace_hide_all(fission_nk_panel_workspace_t *workspace);

void fission_nk_panel_workspace_draw_window_menu(
    struct nk_context *ctx,
    fission_nk_panel_workspace_t *workspace,
    const char *menu_label,
    float menu_width,
    float menu_height,
    const char *reset_button_label,
    fission_nk_panel_workspace_reset_layout_fn reset_layout,
    void *reset_layout_user_data
);

void fission_nk_panel_workspace_draw_panels_menu(
    struct nk_context *ctx,
    fission_nk_panel_workspace_t *workspace,
    const char *menu_label,
    float menu_width,
    float menu_height
);

typedef struct fission_nk_panel_menu_bar_config {
    const char *window_id;
    const char *title_label;
    const char *shortcut_label;
    const char *window_menu_label;
    const char *panels_menu_label;
    const char *reset_button_label;
    float height;
    float window_menu_width;
    float panels_menu_width;
    float title_width;
} fission_nk_panel_menu_bar_config_t;

void fission_nk_panel_workspace_draw_menu_bar(
    struct nk_context *ctx,
    fission_nk_panel_workspace_t *workspace,
    int window_width,
    const fission_nk_panel_menu_bar_config_t *config,
    fission_nk_panel_workspace_reset_layout_fn reset_layout,
    void *reset_layout_user_data
);

#endif
