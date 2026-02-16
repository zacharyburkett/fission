#ifndef FISSION_NUKLEAR_H
#define FISSION_NUKLEAR_H

/*
 * Canonical Nuklear include for Fission consumers.
 *
 * Usage:
 *   #define NK_IMPLEMENTATION
 *   #include "fission/nuklear.h"
 *
 * This ensures shared feature flags are applied consistently across tools.
 */
#include "fission/nuklear_features.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wc23-extensions"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
#include <nuklear.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
