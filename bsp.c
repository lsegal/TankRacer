#include "common.h"
#include "bsp.h"

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glext.h"
#include "glut.h"
#include "jpeg.h"
#include "extensions/ARB_multitexture_extension.h"

static int whiteLightmap;

/* BSP up axis is Z, flip this to Y */
static void bsp_point_swivel(float point[3]) {
	float tmp = point[1];
	point[1] = point[2];
	point[2] = -tmp;
}

/* Reads the contents of a lump into a buffer */
static void bsp_readentry(FILE *file, direntry *entry, void *buffer) {
	fseek(file, entry->offset, SEEK_SET);
	fread(buffer, entry->length, 1, file);
}

static void bsp_draw_mesh(bspfile *bsp, face *cface) {
	glVertexPointer(3, GL_FLOAT, sizeof(vertex), &bsp->data.vertexes[cface->vertex].position);
	//glNormalPointer(GL_FLOAT, sizeof(vertex), &bsp->data.vertexes[cface->vertex].normal);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), &bsp->data.vertexes[cface->vertex].texcoord[0]);

	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), &bsp->data.vertexes[cface->vertex].texcoord[1]);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	glDrawElements(GL_TRIANGLES, cface->n_meshverts, GL_UNSIGNED_INT, &bsp->data.meshverts[cface->meshvert]);
}

static void bsp_draw_patch(bspfile *bsp, patchlist *patches) {
	int i, x;
	tesselpatch *patch;

	for (i = 0; i < patches->n_patches; i++) {
		patch = &patches->patches[i];

		//glNormalPointer(GL_FLOAT, sizeof(vertex), &patch->vertexes[0].normal);

		glVertexPointer(3, GL_FLOAT, sizeof(vertex), &patch->vertexes[0].position);

		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), &patch->vertexes[0].texcoord[0]);
		
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), &patch->vertexes[0].texcoord[1]);
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		
		for (x = 0; x < patch->tesslevel; x++) {
			glDrawElements(GL_TRIANGLE_STRIP, 2 * (patch->tesslevel + 1), 
				GL_UNSIGNED_INT, &patch->indexes[x * 2 * (patch->tesslevel + 1)]);
		}
	}
}

void bsp_draw(bspfile *bsp) {
	int i;
	face *cface;

	glFrontFace(GL_CW);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_NORMAL_ARRAY);

	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	glPushAttrib(GL_TEXTURE_BIT);

	for (i = 0; i < bsp->data.n_faces; i++) {
		cface = &bsp->data.faces[i];

		if (cface->texture != -1) {
			glBindTexture(GL_TEXTURE_2D, bsp->texture_indexes[cface->texture]);

			/* This face is alpha transparent wherever there is black in the image */
			if (bsp->data.textures[cface->texture].flags > 0) {
				glDisable(GL_CULL_FACE);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
			}
		}

		/* Lightmap drawing */
		glActiveTextureARB(GL_TEXTURE1_ARB);
		if (cface->lm_index >= 0) {
			glBindTexture(GL_TEXTURE_2D, bsp->lightmap_indexes[cface->lm_index]);
		}
		else {
			glBindTexture(GL_TEXTURE_2D, whiteLightmap);
		}
		glActiveTextureARB(GL_TEXTURE0_ARB);

		switch (cface->type) {
			case FACE_POLYGON:
			case FACE_MESH:
				bsp_draw_mesh(bsp, cface);
				break;
			case FACE_PATCH:
				bsp_draw_patch(bsp, bsp->tesselpatches[i]);
				break;
		}

		if (cface->texture != -1 && bsp->data.textures[cface->texture].flags > 0) {
			glDisable(GL_BLEND);
			glEnable(GL_CULL_FACE);
		}
	}

	glPopAttrib();

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_NORMAL_ARRAY);

	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glFrontFace(GL_CCW);
}

static void bsp_load_textures(bspfile *bsp) {
	int x;
	char full_texname[70];

	for (x = 0; x < bsp->data.n_textures; x++) {
		/* Build filename */
		strncpy(full_texname, bsp->data.textures[x].name, 64);
		strncat(full_texname, ".jpg", 4);

		/* Load jpeg texture */
		bsp->texture_indexes[x] = load_texture_jpeg(full_texname);
	}
}

static void bsp_load_lightmaps(bspfile *bsp) {
	int i, x, y, z;
	float c, scale, temp;
	ubyte white[3] = { 220, 220, 220 };

	glPushAttrib(GL_TEXTURE_BIT);

	/* Generate lightmap texture from lightmap */
	for (i = 0; i < bsp->data.n_lightmaps; i++) {
		/* Increase brightness of lightmap */
		for (y = 0; y < 128; y++) {
			for (x = 0; x < 128; x++) {
				ubyte *n = bsp->data.lightmaps[i].map[y][x];

				for (z = 0; z < 3; z++) {
					c = n[z];
					c *= 2.5f / 255.0f;

					scale = 1.0f;
					temp  = scale / c;
					if (c > 1.0f && temp < scale) scale = temp;
					
					c *= 255.0f * scale;
					n[z] = (ubyte)c;
				}
			}
		}

		glGenTextures(1, &bsp->lightmap_indexes[i]);
		glBindTexture(GL_TEXTURE_2D, bsp->lightmap_indexes[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, 
			128, 128, GL_RGB, GL_UNSIGNED_BYTE, bsp->data.lightmaps[i].map);
	}

	/* Generate a white lightmap for unlit textures */
	glGenTextures(1, &whiteLightmap);
	glBindTexture(GL_TEXTURE_2D, whiteLightmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &white);

	glPopAttrib();
}

static void bsp_load_vertexes(bspfile *bsp) {
	int i;

	/* Fix vertex orientation */
	for (i = 0; i < bsp->data.n_vertexes; i++) {
		bsp_point_swivel(bsp->data.vertexes[i].position);
		bsp_point_swivel(bsp->data.vertexes[i].normal);

		/* Scale values down by 64x */
		bsp->data.vertexes[i].position[0] /= 64;
		bsp->data.vertexes[i].position[1] /= 64;
		bsp->data.vertexes[i].position[2] /= 64;
	}
}

static void bsp_vertex_scale(vertex *vert, float scale, vertex *out) {
	int i;
	
	for (i = 0; i < 3; i++) out->position[i] = vert->position[i] * scale;
	for (i = 0; i < 2; i++) {
		out->texcoord[0][i] = vert->texcoord[0][i] * scale;
		out->texcoord[1][i] = vert->texcoord[1][i] * scale;
	}
}

static void bsp_vertex_add(vertex *a, vertex *b, vertex *out) {
	int i;
	
	for (i = 0; i < 3; i++) out->normal[i] = a->normal[i] + b->normal[i];
	for (i = 0; i < 3; i++) out->position[i] = a->position[i] + b->position[i];
	for (i = 0; i < 2; i++) {
		out->texcoord[0][i] = a->texcoord[0][i] + b->texcoord[0][i];
		out->texcoord[1][i] = a->texcoord[1][i] + b->texcoord[1][i];
	}
}

/* Use control points in tesselpatch to generate vertexes */
static void bsp_tesselate_patch(bspfile *bsp, tesselpatch *patch, int level) {
	int vsize, isize, i, v;
	float px, py;
	vertex temp[3], t2[3];

	/* We already have this generated data if level hasn't changed since last call */
	if (patch->vertexes && patch->tesslevel == level) return;

	memset(&temp, 0, sizeof(vertex) * 3);
	memset(&t2, 0, sizeof(vertex) * 3);
	patch->tesslevel = level;
	
	/* Allocate (or reallocate) room for tesselated vertices (and indices) */
	vsize = (level+1) * (level+1) * sizeof(vertex);
	isize = level * (level + 1) * 2 * sizeof(int);
	if (!patch->vertexes) {
		patch->vertexes = malloc(vsize);
		patch->indexes  = malloc(isize);
	}
	else {
		realloc(patch->vertexes, vsize);
		realloc(patch->indexes,  isize);
	}

	for (i = 0; i <= level; i++) {
		px = (float)i / level;

		bsp_vertex_scale(&patch->ctlpoints[0], (1.0f - px) * (1.0f - px), &temp[0]);
		bsp_vertex_scale(&patch->ctlpoints[3], (1.0f - px) * px * 2, &temp[1]);
		bsp_vertex_scale(&patch->ctlpoints[6], px * px, &temp[2]);

		memset(&patch->vertexes[i], 0, sizeof(vertex));
		bsp_vertex_add(&temp[0], &temp[1], &temp[0]);
		bsp_vertex_add(&temp[0], &temp[2], &patch->vertexes[i]);
	}

	for (v = 1; v <= level; v++) {
		py = (float)v / level;

		bsp_vertex_scale(&patch->ctlpoints[0], (1.0f - py) * (1.0f - py), &t2[0]);
		bsp_vertex_scale(&patch->ctlpoints[1], (1.0f - py) * py * 2, &t2[1]);
		bsp_vertex_scale(&patch->ctlpoints[2], py * py, &t2[2]);
		bsp_vertex_add(&t2[0], &t2[1], &t2[0]);
		bsp_vertex_add(&t2[0], &t2[2], &temp[0]);

		bsp_vertex_scale(&patch->ctlpoints[3], (1.0f - py) * (1.0f - py), &t2[0]);
		bsp_vertex_scale(&patch->ctlpoints[4], (1.0f - py) * py * 2, &t2[1]);
		bsp_vertex_scale(&patch->ctlpoints[5], py * py, &t2[2]);
		bsp_vertex_add(&t2[0], &t2[1], &t2[0]);
		bsp_vertex_add(&t2[0], &t2[2], &temp[1]);

		bsp_vertex_scale(&patch->ctlpoints[6], (1.0f - py) * (1.0f - py), &t2[0]);
		bsp_vertex_scale(&patch->ctlpoints[7], (1.0f - py) * py * 2, &t2[1]);
		bsp_vertex_scale(&patch->ctlpoints[8], py * py, &t2[2]);
		bsp_vertex_add(&t2[0], &t2[1], &t2[0]);
		bsp_vertex_add(&t2[0], &t2[2], &temp[2]);

		for(i = 0; i <= level; i++) {
			px = (float)i / level;

			bsp_vertex_scale(&temp[0], (1.0f - px) * (1.0f - px), &t2[0]);
			bsp_vertex_scale(&temp[1], (1.0f - px) * px * 2, &t2[1]);
			bsp_vertex_scale(&temp[2], px * px, &t2[2]);

			memset(&patch->vertexes[v*(level+1)+i], 0, sizeof(vertex));
			bsp_vertex_add(&t2[0], &t2[1], &t2[0]);
			bsp_vertex_add(&t2[0], &t2[2], &patch->vertexes[v*(level+1)+i]);
		}
	}

	/* Generate indices */
	for (v = 0; v < level; v++) {
		for(i = 0; i <= level; i++) {
			patch->indexes[(v * (level + 1) + i) * 2]     = (v + 1) * (level + 1) + i;
			patch->indexes[(v * (level + 1) + i) * 2 + 1] = v * (level + 1) + i;
		}
	}
}

static void bsp_load_faces(bspfile *bsp) {
	int i, x, y, width, height, row, col;
	face *cface;
	tesselpatch *patch;

	/* Find patches and tesselate them */
	bsp->tesselpatches = malloc(sizeof(patchlist*) * bsp->data.n_faces);
	for (i = 0; i < bsp->data.n_faces; i++) {
		cface = &bsp->data.faces[i];

		if (cface->type == FACE_PATCH) {
			/* Found a patch, initialize memory for tesselation */
			width  = cface->size[0];
			height = cface->size[1];

			bsp->tesselpatches[i] = malloc(sizeof(patchlist));
			bsp->tesselpatches[i]->n_patches = (width-1)/2 * (height-1)/2;
			bsp->tesselpatches[i]->patches = malloc(width * height * sizeof(tesselpatch));
			memset(bsp->tesselpatches[i]->patches, 0, width * height * sizeof(tesselpatch));

			/* This algorithm is a mouthful. 
			 * Loop through all the patches in the list and copy vertexes 
			 */
			for (y = 0; y < height; y++) {
				for (x = 0; x < width; x++) {
					patch = &bsp->tesselpatches[i]->patches[y*width+x];

					/* Copy over each control point one by one */
					for (row = 0; row < 3; row++) {
						for (col = 0; col < 3; col++) {
							/* Copy the vertex from the vertex offset in the 
							 * vertexes list to the control point 
							 */
							memcpy(&patch->ctlpoints[row*3+col], &bsp->data.vertexes[cface->vertex+
								(y*2*width+x*2)+row*width+col], sizeof(vertex));
						}
					}

					bsp_tesselate_patch(bsp, patch, 10);
				}
			}
		}
	}
}

bspfile *bsp_load(char *filename) {
	int i, len;
	bspfile *bsp;
	FILE *fp = fopen(filename, "rb");

	if (!fp) return NULL;

	bsp = malloc(sizeof(bspfile));
	memset(bsp, 0, sizeof(bspfile));
	fread(&bsp->header, sizeof(bspheader), 1, fp);

	for (i = 0; i < NUM_BSP_LUMPS; i++) {
		len = bsp->header.direntries[i].length;

		if (i == TEXTURES) {
			bsp->data.textures = malloc(len);
			bsp->data.n_textures = len / sizeof(texture);
			bsp_readentry(fp, &bsp->header.direntries[i], bsp->data.textures);			

			bsp->texture_indexes = malloc(bsp->data.n_textures * sizeof(int));
			bsp_load_textures(bsp);
		}
		else if (i == LIGHTMAPS) {
			bsp->data.lightmaps = malloc(len);
			bsp->data.n_lightmaps = len / sizeof(lightmap);
			bsp_readentry(fp, &bsp->header.direntries[i], bsp->data.lightmaps);			

			bsp->lightmap_indexes = malloc(bsp->data.n_lightmaps * sizeof(int));
			bsp_load_lightmaps(bsp);
		}
		else if (i == VERTEXES) {
			bsp->data.vertexes = malloc(len);
			bsp->data.n_vertexes = len / sizeof(vertex);
			bsp_readentry(fp, &bsp->header.direntries[i], bsp->data.vertexes);
			bsp_load_vertexes(bsp);
		}
		else if (i == FACES) {
			bsp->data.faces = malloc(len);
			bsp->data.n_faces = len / sizeof(face);
			bsp_readentry(fp, &bsp->header.direntries[i], bsp->data.faces);
			bsp_load_faces(bsp);
		}
		else if (i == MESHVERTS) {
			bsp->data.meshverts = malloc(len);
			bsp->data.n_meshverts = len / sizeof(meshvert);
			bsp_readentry(fp, &bsp->header.direntries[i], bsp->data.meshverts);
		}
	}
	fclose(fp);

	return bsp;
}

void bsp_free(bspfile *bsp) {
	if (bsp->data.brushes) free(bsp->data.brushes);
	if (bsp->data.brushsides) free(bsp->data.brushsides);
	free(bsp);
}