#ifndef FISSION_NUKLEAR_RENDER_H
#define FISSION_NUKLEAR_RENDER_H

struct nk_image;

typedef struct fission_nk_texture {
    unsigned int id;
    int width;
    int height;
} fission_nk_texture_t;

typedef enum fission_nk_texture_sampling {
    FISSION_NK_TEXTURE_SAMPLING_PIXEL_ART = 0,
    FISSION_NK_TEXTURE_SAMPLING_SMOOTH = 1
} fission_nk_texture_sampling_t;

void fission_nk_texture_init(fission_nk_texture_t *texture);
void fission_nk_texture_destroy(fission_nk_texture_t *texture);

int fission_nk_texture_upload_rgba8(
    fission_nk_texture_t *texture,
    int width,
    int height,
    const unsigned char *pixels,
    fission_nk_texture_sampling_t sampling
);

int fission_nk_texture_upload_rgba8_image(
    fission_nk_texture_t *texture,
    int width,
    int height,
    const unsigned char *pixels,
    fission_nk_texture_sampling_t sampling,
    struct nk_image *out_image
);

#endif
