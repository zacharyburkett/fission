#include "fission/nuklear_render.h"

#include <limits.h>
#include <stdarg.h>
#include <stddef.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#if defined(__APPLE__)
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "fission/nuklear.h"

static GLint fission_nk_mag_filter_from_sampling(fission_nk_texture_sampling_t sampling)
{
    if (sampling == FISSION_NK_TEXTURE_SAMPLING_SMOOTH) {
        return GL_LINEAR;
    }
    return GL_NEAREST;
}

void fission_nk_texture_init(fission_nk_texture_t *texture)
{
    if (texture == NULL) {
        return;
    }

    texture->id = 0u;
    texture->width = 0;
    texture->height = 0;
}

void fission_nk_texture_destroy(fission_nk_texture_t *texture)
{
    GLuint texture_id;

    if (texture == NULL) {
        return;
    }

    texture_id = (GLuint)texture->id;
    if (texture_id != 0u) {
        glDeleteTextures(1, &texture_id);
    }
    fission_nk_texture_init(texture);
}

int fission_nk_texture_upload_rgba8(
    fission_nk_texture_t *texture,
    int width,
    int height,
    const unsigned char *pixels,
    fission_nk_texture_sampling_t sampling
)
{
    GLuint texture_id;

    if (texture == NULL || pixels == NULL || width <= 0 || height <= 0) {
        return 0;
    }

    texture_id = (GLuint)texture->id;
    if (texture_id == 0u) {
        glGenTextures(1, &texture_id);
        if (texture_id == 0u) {
            return 0;
        }
        texture->id = (unsigned int)texture_id;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, fission_nk_mag_filter_from_sampling(sampling));
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (texture->width != width || texture->height != height) {
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            (GLsizei)width,
            (GLsizei)height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            pixels
        );
        texture->width = width;
        texture->height = height;
    } else {
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0,
            0,
            (GLsizei)width,
            (GLsizei)height,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            pixels
        );
    }

    glBindTexture(GL_TEXTURE_2D, 0u);
    return 1;
}

int fission_nk_texture_upload_rgba8_image(
    fission_nk_texture_t *texture,
    int width,
    int height,
    const unsigned char *pixels,
    fission_nk_texture_sampling_t sampling,
    struct nk_image *out_image
)
{
    if (out_image == NULL) {
        return 0;
    }

    if (!fission_nk_texture_upload_rgba8(texture, width, height, pixels, sampling)) {
        return 0;
    }

    if (texture->id > (unsigned int)INT_MAX) {
        return 0;
    }

    *out_image = nk_image_id((int)texture->id);
    return 1;
}
