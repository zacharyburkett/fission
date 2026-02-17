# Fission

Fission is a lightweight C utility layer for building Nuklear-based tooling UIs.

> Stability notice
> This repository is pre-1.0 and not stable. APIs, panel workspace behavior, and helper conventions may change between commits.

## What Is Implemented

Core library target:

- `fission::fission`

Optional OpenGL helper target:

- `fission::nuklear_render` (built when OpenGL is found)

Current utility surface:

- Shared Nuklear feature-flag include entrypoint (`fission/nuklear.h`)
- Theme and window ID helpers
- Rect/intersection math helpers
- Splitter and panel-host utilities
- Multi-workspace tabbed panel layouts
- RGBA texture upload helpers for `nk_image` rendering (OpenGL target)

## Build

```sh
cmake -S . -B build
cmake --build build
```

Dependencies:

- `nuklear.h` is required
- OpenGL is optional (only for `fission::nuklear_render`)

By default, Fission can auto-fetch Nuklear if missing.

Offline/pinned dependency example:

```sh
cmake -S . -B build \
  -DFISSION_NUKLEAR_AUTO_FETCH=OFF \
  -DFISSION_NUKLEAR_INCLUDE_DIR=/absolute/path/to/nuklear
cmake --build build
```

## CMake Options

- `FISSION_NUKLEAR_AUTO_FETCH=ON|OFF`
- `FISSION_NUKLEAR_INCLUDE_DIR=/path/to/nuklear`

## Consumer Integration

From source:

```cmake
add_subdirectory(/absolute/path/to/fission ${CMAKE_BINARY_DIR}/_deps/fission EXCLUDE_FROM_ALL)
target_link_libraries(my_target PRIVATE fission::fission)
if(TARGET fission::nuklear_render)
    target_link_libraries(my_target PRIVATE fission::nuklear_render)
endif()
```

Installed package export is also generated (`fissionTargets.cmake`).

## Public Headers

- `include/fission/nuklear_features.h`
- `include/fission/nuklear.h`
- `include/fission/nuklear_ui.h`
- `include/fission/nuklear_render.h`
- `include/fission/nuklear_panels.h`
- `include/fission/ui.h`
