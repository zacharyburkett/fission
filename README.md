# Fission

Fission is a lightweight C library of utilities for building Nuklear UIs.

It provides:

- A shared Nuklear feature-flag surface
- Opinionated UI helpers (theme, window IDs, rect helpers, splitters)
- OpenGL texture upload helpers for `nk_image` rendering

## Design Goals

- Standalone-first: usable in any C/C++ project
- Small API surface with minimal setup
- Consistent behavior across multiple tools/apps
- Practical defaults with room for customization

## Public Headers

- `include/fission/nuklear_features.h`
- `include/fission/nuklear.h`
- `include/fission/nuklear_ui.h`
- `include/fission/nuklear_render.h`
- `include/fission/nuklear_panels.h`
- `include/fission/ui.h`

`nuklear.h`:

- Canonical Fission include for Nuklear
- Applies shared `NK_INCLUDE_*` flags automatically
- Supports `#define NK_IMPLEMENTATION` before include

`nuklear_ui.h`:

- `fission_nk_apply_theme`
- `fission_nk_make_window_id`
- `fission_nk_rect_translate`
- `fission_nk_rect_intersection`
- `fission_nk_draw_splitter_overlay`

`nuklear_render.h`:

- `fission_nk_texture_init`
- `fission_nk_texture_destroy`
- `fission_nk_texture_upload_rgba8`
- `fission_nk_texture_upload_rgba8_image`

`nuklear_panels.h`:

- Docked/floating panel workspace (`fission_nk_panel_workspace_t`)
- Panel registration, layout, splitters, and panel menus
- Multi-workspace tab state (`fission_nk_panel_workspace_tabs_t`)
- Workspace tab menu bar rendering and tab operations (create/switch/rename/move/remove)

## Dependencies

- Nuklear headers (`nuklear.h`) are required.
- OpenGL is optional and only needed for `fission::nuklear_render`.

## Build

```sh
cmake -S . -B build
cmake --build build
```

Useful CMake options:

- `FISSION_NUKLEAR_AUTO_FETCH=ON|OFF` (default: `ON`)
- `FISSION_NUKLEAR_INCLUDE_DIR=/path/to/nuklear` (directory containing `nuklear.h`)

Offline example:

```sh
cmake -S . -B build \
  -DFISSION_NUKLEAR_AUTO_FETCH=OFF \
  -DFISSION_NUKLEAR_INCLUDE_DIR=/absolute/path/to/nuklear
cmake --build build
```

## CMake Targets

- `fission::fission` (always available)
- `fission::nuklear_render` (available when OpenGL is found)

Add from source:

```cmake
add_subdirectory(/absolute/path/to/fission ${CMAKE_BINARY_DIR}/_deps/fission EXCLUDE_FROM_ALL)
target_link_libraries(my_ui_target PRIVATE fission::fission)
if(TARGET fission::nuklear_render)
    target_link_libraries(my_ui_target PRIVATE fission::nuklear_render)
endif()
```

## Usage

Use the Fission Nuklear entrypoint:

```c
#define NK_IMPLEMENTATION
#include "fission/nuklear.h"
```

Apply theme and build stable window IDs:

```c
#include "fission/nuklear_ui.h"

fission_nk_apply_theme(ctx);
fission_nk_make_window_id(window_id, sizeof(window_id), "my_app.tools", "preview");
```

Upload RGBA8 pixels and render as `nk_image`:

```c
#include "fission/nuklear_render.h"

fission_nk_texture_t texture;
struct nk_image image;

fission_nk_texture_init(&texture);
if (fission_nk_texture_upload_rgba8_image(
        &texture,
        width,
        height,
        pixels_rgba8,
        FISSION_NK_TEXTURE_SAMPLING_PIXEL_ART,
        &image
    )) {
    nk_image(ctx, image);
}
fission_nk_texture_destroy(&texture);
```

## Panel Workspaces and Workspace Tabs

Fission provides a reusable panel host for editor-style UIs:

- `fission_nk_panel_workspace_t` manages docked/floating panel layout
- `fission_nk_panel_workspace_tabs_t` stores multiple independent workspace layouts

Minimal setup:

```c
#include "fission/nuklear_panels.h"

fission_nk_panel_workspace_t workspace;
fission_nk_panel_workspace_tabs_t workspace_tabs;

fission_nk_panel_workspace_init(&workspace);
fission_nk_panel_workspace_tabs_init(&workspace_tabs);

/* Register each panel once. */
fission_nk_panel_workspace_tabs_register_panel(
    &workspace_tabs,
    &workspace,
    &panel_desc
);
```

Per-frame usage:

```c
fission_nk_panel_workspace_tabs_menu_bar_config_t menu_cfg;
memset(&menu_cfg, 0, sizeof(menu_cfg));
menu_cfg.window_id = "__my_editor_menu";
menu_cfg.shortcut_label = "F5 Reload  F6 Pause  F7 Step";
menu_cfg.shortcut_compact_label = "F5/F6/F7";

fission_nk_panel_workspace_tabs_draw_menu_bar(
    ctx,
    &workspace_tabs,
    &workspace,
    window_width,
    &menu_cfg,
    reset_layout_fn,
    reset_layout_user_data
);

fission_nk_panel_workspace_tabs_draw_all(
    &workspace_tabs,
    &workspace,
    ctx,
    window_width,
    window_height
);
```

Shutdown:

```c
fission_nk_panel_workspace_tabs_shutdown(&workspace_tabs);
fission_nk_panel_workspace_shutdown(&workspace);
```
