#include "common.h"
#include "jpeglib.h"
#include "jpeg.h"

int load_texture_jpeg(char *filename) {
	unsigned int format, texture, stride;
	ubyte *data, *ptr;
	FILE *file;
	JSAMPARRAY linebuf;
	struct jpeg_error_mgr err;
	struct jpeg_decompress_struct cinfo;
	
	file = fopen(filename, "rb");	
	if (!file) {
		fprintf(stderr, "Could not load texture '%s'\n", filename);
		return -1;
	}

	/* Setup jpeg */
	cinfo.err = jpeg_std_error(&err);
	cinfo.raw_data_out = TRUE;
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, file);
	jpeg_read_header(&cinfo, TRUE);
	
	/* Verify that image is either 24 or 32 bitdepth */
	if (cinfo.num_components == 3) {
		format = GL_RGB;
	}
	else if (cinfo.num_components == 4) {
		format = GL_RGBA;
	}
	else {
		fprintf(stderr, "Unknown bit depth %d for texture '%s'\n", cinfo.num_components<<3, filename);
		return -1;
	}

	/* Read image data */
	jpeg_start_decompress(&cinfo);

	data = malloc(cinfo.image_height * cinfo.image_width * cinfo.num_components);
	if (!data) {
		fprintf(stderr, "Could not allocate memory for texture '%s'\n", filename);
		return -1;
	}

	stride = cinfo.output_width * cinfo.output_components;
	linebuf = (*cinfo.mem->alloc_sarray)
		((j_common_ptr)&cinfo, JPOOL_IMAGE, stride, 1);

	ptr = data;
	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, linebuf, 1);
		memcpy(ptr, linebuf[0], stride);
		ptr += stride;
	}

	jpeg_finish_decompress(&cinfo);
	fclose(file);

	/* Get a texture */
    glGenTextures(1, (GLuint*)&texture);

	/* Set texture parameters */
	glPushAttrib(GL_TEXTURE_BIT);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	/* Build texture */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	gluBuild2DMipmaps(GL_TEXTURE_2D, cinfo.num_components, 
		cinfo.image_width, cinfo.image_height, format, GL_UNSIGNED_BYTE, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPopAttrib();

	jpeg_destroy_decompress(&cinfo);

	free(data);

	return texture;
}