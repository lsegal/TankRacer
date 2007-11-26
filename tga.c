#include "common.h"
#include "tgalib.h"

int load_texture_tga(char *filename) {
	int texture, format;

	tImageTGA *img = LoadTGA(filename);
	if (img == NULL) {
		fprintf(stderr, "Could not load texture '%s'\n", filename);
		return -1;
	}

	if (img->channels == 3) {
		format = GL_RGB;
	}
	else if (img->channels == 4) {
		format = GL_RGBA;
	}
	else {
		fprintf(stderr, "Unknown bit depth %d for texture '%s'\n", img->channels<<3, filename);
		return -1;
	}

	/* Get a texture */
    glGenTextures(1, &texture);

	/* Set texture parameters */
	glPushAttrib(GL_TEXTURE_BIT);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	/* Build texture */
	gluBuild2DMipmaps(GL_TEXTURE_2D, img->channels, 
		img->sizeX, img->sizeY, format, GL_UNSIGNED_BYTE, img->data);
	glPopAttrib();

	free(img);

	return texture;
}