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
- `include/fission/nuklear_ui.h`
- `include/fission/nuklear_render.h`

`nuklear_features.h`:

- Defines shared `NK_INCLUDE_*` flags
- Must be included before `nuklear.h`

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

Include shared Nuklear features before `nuklear.h`:

```c
#include "fission/nuklear_features.h"
#define NK_IMPLEMENTATION
#include "nuklear.h"
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
