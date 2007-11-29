#include "common.h"

/* Loads an RGBx8 raw file and returns the texture number or -1 if it could not load one. */
GLuint load_texture_raw(const char *filename, int width, int height, int repeat) {
    unsigned char *buf;
    GLuint texture;
    FILE *fp;

    fp = fopen(filename, "rb");
	if (!fp) {
		fprintf(stderr, "Could not load texture '%s'\n", filename);
		return -1;
	}

	/* Load the buffer */
    buf = malloc(width * height * 3);
    fread(buf, width * height * 3, 1, fp);
    fclose(fp);

	/* Get a texture */
    glGenTextures(1, &texture);

	/* Set texture parameters */
	glPushAttrib(GL_TEXTURE_BIT);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat);

	/* Build texture */
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, buf);
	glPopAttrib();

    free(buf);

    return texture;
}