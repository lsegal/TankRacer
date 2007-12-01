#include "common.h"
#include "bsp.h"

#ifdef WIN32
#	include "extensions/ARB_multitexture_extension.h"
#endif

static int whiteLightmap;

/* BSP up axis is Z, flip this to Y */
static void bsp_point_swivel(float point[3]) {
	float tmp = point[1];
	point[1] = point[2];
	point[2] = -tmp;
}

static void bsp_point_scwivel(float vert[3]) {
	int i;
	bsp_point_swivel(vert);
	for (i = 0; i < 3; i++) vert[i] /= BSP_SCALE;
}

static void bsp_fix_bounding_box(int box[3]) {
	int i;
	float tmp;
	for (i = 0; i < 3; i++) box[i] /= BSP_SCALE;
	tmp = box[1];
	box[1] = box[2];
	box[2] = -tmp;
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

void bsp_draw_faces(bspfile *bsp, int *facelist, int numfaces) {
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

	for (i = 0; i < numfaces; i++) {
		cface = &bsp->data.faces[facelist[i]];

		if (cface->texture < 0) continue; /* Don't draw unloaded textures */

		if (bsp->data.textures[cface->texture].flags & 4)
			continue; /* Don't draw skies either. We'll do this after */

		/* This face is alpha transparent wherever there is black in the image */
		if (bsp->data.textures[cface->texture].flags & 32) {
			glDisable(GL_CULL_FACE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
		}

		glBindTexture(GL_TEXTURE_2D, bsp->texture_indexes[cface->texture]);

		/* Lightmap drawing */
		glActiveTextureARB(GL_TEXTURE1_ARB);
		if (cface->lm_index >= 0 && !(bsp->data.textures[cface->texture].flags & 4)) {
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
				bsp_draw_patch(bsp, bsp->tesselpatches[facelist[i]]);
				break;
		}

		if (bsp->data.textures[cface->texture].flags & 32) {
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
	int x, tex;
	char full_texname[70];

	for (x = 0; x < bsp->data.n_textures; x++) {
		/* Attempt to load texture as jpeg */
		strncpy(full_texname, (char *)&bsp->data.textures[x].name, 64);
		strncat(full_texname, ".jpg", 4);
		tex = load_texture_jpeg(full_texname);

		if (tex == -1) { /* Attempt to load texture as .tga */
			strncpy(full_texname, (char *)&bsp->data.textures[x].name, 64);
			strncat(full_texname, ".tga", 4);
			tex = load_texture_tga(full_texname);
		}

		if (tex == -1) {
			fprintf(stderr, "Could not load texture '%s'\n", bsp->data.textures[x].name);
		}
		bsp->texture_indexes[x] = tex;
	}
}

static void bsp_load_lightmaps(bspfile *bsp) {
	int i, x, y, z;
	float c, scale, temp;
	ubyte white[3] = { 255, 255, 255 };

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

		glGenTextures(1, (GLuint*)&bsp->lightmap_indexes[i]);
		glBindTexture(GL_TEXTURE_2D, bsp->lightmap_indexes[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, 
			128, 128, GL_RGB, GL_UNSIGNED_BYTE, bsp->data.lightmaps[i].map);
	}

	/* Generate a white lightmap for unlit textures */
	glGenTextures(1, (GLuint*)&whiteLightmap);
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

	/* Fix vertex orientation and scale */
	for (i = 0; i < bsp->data.n_vertexes; i++) {
		bsp_point_scwivel(bsp->data.vertexes[i].position);
		bsp_point_swivel(bsp->data.vertexes[i].normal);
	}
}

static void bsp_load_planes(bspfile *bsp) {
	int i;

	/* Fix vertex orientation and scale */
	for (i = 0; i < bsp->data.n_planes; i++) {
		bsp_point_swivel(bsp->data.planes[i].normal);
		bsp->data.planes[i].dist /= BSP_SCALE;
	}
}

static void bsp_load_leafs(bspfile *bsp) {
	int i;

	/* Fix vertex orientation and scale */
	for (i = 0; i < bsp->data.n_leafs; i++) {
		bsp_fix_bounding_box(bsp->data.leafs[i].mins);
		bsp_fix_bounding_box(bsp->data.leafs[i].maxs);
	}
}

static void bsp_load_nodes(bspfile *bsp) {
	int i;

	/* Fix vertex orientation and scale */
	for (i = 0; i < bsp->data.n_nodes; i++) {
		bsp_fix_bounding_box(bsp->data.nodes[i].mins);
		bsp_fix_bounding_box(bsp->data.nodes[i].maxs);
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

	/* Get ready to tesselate some patches */
	bsp->tesselpatches = malloc(sizeof(patchlist*) * bsp->data.n_faces);
	memset(bsp->tesselpatches, 0, sizeof(patchlist*) * bsp->data.n_faces);
	
	for (i = 0; i < bsp->data.n_faces; i++) {
		cface = &bsp->data.faces[i];

		/* Fix normals */
		bsp_point_swivel(cface->normal);

		/* Find patches and tesselate them */
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

/* Reads the contents of a lump into a buffer */
static void bsp_readentry(FILE *file, direntry *entry, void *buffer) {
	fseek(file, entry->offset, SEEK_SET);
	fread(buffer, entry->length, 1, file);
}

/* Finds the leaf that a point lies in */
leaf *bsp_find_leaf(bspfile *bsp, float origin[3]) {
	int index = 0, type;
	node *cnode;
	plane *cplane;
	
	while (index >= 0) {
		cnode = &bsp->data.nodes[index];
		cplane = &bsp->data.planes[cnode->plane];

		type = vec3f_classify(origin, cplane->normal, cplane->dist);
		index = type >= 0 ? cnode->front : cnode->back;
	}

	return &bsp->data.leafs[~index];
}

/* Tests whether a leaf is visible from another leaf */
int bsp_leaf_visible(bspfile *bsp, leaf *visLeaf, leaf *testLeaf) {
	int i;
	if (!bsp->data.vis) return 1;
	if (visLeaf->cluster < 0) return 0;
	i = (visLeaf->cluster * bsp->data.vis->sz_vecs) + (testLeaf->cluster >> 3);
	return bsp->data.vis[0].vecs[i] & (1 << (testLeaf->cluster & 7));
}

/* Tests if a line passes through a brush */
plane *bsp_simple_collision(bspfile *bsp, float p1[3], float p2[3], leaf *leaf1, leaf *leaf2) {
	int i, j, n, count = 2;
	float d1, d2;
	leaf *leaves[2];
	leafbrush *lbrush;
	brush *cbrush;
	plane *cplane;

	if (!leaf1) leaf1 = bsp_find_leaf(bsp, p1);
	if (!leaf2) leaf2 = bsp_find_leaf(bsp, p2);
	leaves[0] = leaf1; leaves[1] = leaf2;

	if (leaf1 == leaf2) return NULL;

	for (n = 0; n < count; n++) {
		for (i = 0; i < leaves[n]->n_leafbrushes; i++) {
			lbrush = &bsp->data.leafbrushes[leaves[n]->leafbrush+i];
			cbrush = &bsp->data.brushes[lbrush->brush];
			if (!(bsp->data.textures[cbrush->texture].contents & 1)) continue;

			for (j = 0; j < cbrush->n_brushsides; j++) {
				cplane = &bsp->data.planes[bsp->data.brushsides[cbrush->brushside+j].plane];

				d1 = vec3f_classify(p1, cplane->normal, cplane->dist);
				d2 = vec3f_classify(p2, cplane->normal, cplane->dist);
				if (!d1 || !d2 || (d1 != d2 && d1 >= 0)) {
					printf("Collision with %s %s\n", bsp->data.textures[cbrush->texture].name,
						(count == 2 ? "(through two leaves)" : ""));
					return cplane;
				}
			}
		}
	}

	return NULL;
}

/* Checks if a point lies in a concave face */
static int bsp_point_in_face(bspfile *bsp, float p[3], face *cface) {
	int i;
	float tmp[3], lasttmp[3], firsttmp[3], angle = 0.0f;

	/* Algorithm ref: http://www.gamespp.com/algorithms/CollisionDetectionTutorial.html */
	for (i = 0; i < cface->n_vertexes; i++) {
		vec3f_sub(bsp->data.vertexes[cface->vertex+i].position, p, tmp);
		vec3f_norm(tmp);

		if (i > 0) {
			angle += acos(vec3f_dot(tmp, lasttmp));
		}
		else {
			vec3f_set(tmp, firsttmp);
		}
		vec3f_set(tmp, lasttmp);
	}
	angle += acos(vec3f_dot(tmp, firsttmp));

	return fabs(angle - 2*PI) < 4.0f;
}

/* Tests if a line segment passes through a face */
face *bsp_face_collision(bspfile *bsp, float p1[3], float dir[3]) {
	int i, n, num = 2;
	leaf *leaves[2];
	face *cface;
	float tmp[3], p2[3], dist;

	/* Get leaves */
	vec3f_add(p1, dir, p2);
	leaves[0] = bsp_find_leaf(bsp, p1);
	leaves[1] = bsp_find_leaf(bsp, p2);
	if (leaves[0] == leaves[1]) num = 1;

	/* Setup parametric equation to get point on plane */
	vec3f_norm(dir);

	for (n = 0; n < num; n++) { 
		for (i = 0; i < leaves[n]->n_leaffaces; i++) {
			cface = &bsp->data.faces[bsp->data.leaffaces[leaves[n]->leafface+i].face];

			if (vec3f_dot(cface->normal, dir) >= 0) continue; /* Face is back facing */

			vec3f_sub(bsp->data.vertexes[cface->vertex].position, p1, tmp);
			dist = -vec3f_dot(cface->normal, tmp) / vec3f_dot(cface->normal, dir);
			vec3f_scale(dir, dist, tmp);
			vec3f_add(p1, tmp, tmp);

			if (bsp_point_in_face(bsp, tmp, cface)) {
	#ifdef _PRINTDEBUG
				printf("Collision with %s\n", bsp->data.textures[cface->texture].name);
	#endif
				return cface;
			}
		}
	}
	return NULL;
}

lightvol *bsp_lightvol(bspfile *bsp, float pos[3]) {
	/* Max grid values */
	int nx = floor(bsp->data.models[0].maxs[0] / 64) - ceil(bsp->data.models[0].mins[0] / 64) + 1;
	int ny = floor(bsp->data.models[0].maxs[1] / 64) - ceil(bsp->data.models[0].mins[1] / 64) + 1;
	int nz = floor(bsp->data.models[0].maxs[2] / 128) - ceil(bsp->data.models[0].mins[2] / 128) + 1;

	/* Convert position into a grid value */
	/* Remember to unscwivel these values! */
	int x = floor(((pos[0] * BSP_SCALE) - bsp->data.models[0].mins[0]) / 64);
	int y = floor(((-pos[2] * BSP_SCALE) - bsp->data.models[0].mins[1]) / 64);
	int z = floor(((pos[1] * BSP_SCALE) - bsp->data.models[0].mins[2]) / 128);

	/* Invalid indexes */
	if (x < 0 || y < 0 || z < 0) return NULL;
	if (x >= nx || y >= ny || z >= nz) return NULL;

	return &bsp->data.lightvols[(nx*ny) * z + (nx * y) + x];
}

/* Loads a .bsp file into memory returning a memory address to allocated data */
bspfile *bsp_load(char *filename) {
	int i, len;
	bspfile *bsp;
	direntry *entry;
	FILE *fp = fopen(filename, "rb");

	if (!fp) {
		return NULL;
	}

	bsp = malloc(sizeof(bspfile));
	memset(bsp, 0, sizeof(bspfile));
	fread(&bsp->header, sizeof(bspheader), 1, fp);

	for (i = 0; i < BSP_NUM_LUMPS; i++) {
		entry = &bsp->header.direntries[i];
		len = entry->length;

		if (len == 0) continue; /* Empty lump */

#ifdef _PRINTDEBUG
		printf("Lump %d: offset = 0x%X \t length = 0x%X\n", i, entry->offset, entry->length);
#endif

		if (i == BSP_MODELS) {
			bsp->data.models = malloc(len);
			bsp->data.n_models = len / sizeof(model);
			bsp_readentry(fp, entry, bsp->data.models);
		}
		if (i == BSP_TEXTURES) {
			bsp->data.textures = malloc(len);
			bsp->data.n_textures = len / sizeof(texture);
			bsp_readentry(fp, entry, bsp->data.textures);			

			bsp->texture_indexes = malloc(bsp->data.n_textures * sizeof(int));
			bsp_load_textures(bsp);
		}
		else if (i == BSP_LIGHTMAPS) {
			bsp->data.lightmaps = malloc(len);
			bsp->data.n_lightmaps = len / sizeof(lightmap);
			bsp_readentry(fp, entry, bsp->data.lightmaps);			

			bsp->lightmap_indexes = malloc(bsp->data.n_lightmaps * sizeof(int));
			bsp_load_lightmaps(bsp);
		}
		else if (i == BSP_LIGHTVOLS) {
			bsp->data.lightvols = malloc(len);
			bsp->data.n_lightvols = len / sizeof(lightvol);
			bsp_readentry(fp, entry, bsp->data.lightvols);			
		}
		else if (i == BSP_VERTEXES) {
			bsp->data.vertexes = malloc(len);
			bsp->data.n_vertexes = len / sizeof(vertex);
			bsp_readentry(fp, entry, bsp->data.vertexes);
			bsp_load_vertexes(bsp);
		}
		else if (i == BSP_FACES) {
			bsp->data.faces = malloc(len);
			bsp->data.n_faces = len / sizeof(face);
			bsp_readentry(fp, entry, bsp->data.faces);
			bsp_load_faces(bsp);
		}
		else if (i == BSP_MESHVERTS) {
			bsp->data.meshverts = malloc(len);
			bsp->data.n_meshverts = len / sizeof(meshvert);
			bsp_readentry(fp, entry, bsp->data.meshverts);
		}
		else if (i == BSP_LEAFS) {
			bsp->data.leafs = malloc(len);
			bsp->data.n_leafs = len / sizeof(leaf);
			bsp_readentry(fp, entry, bsp->data.leafs);
			bsp_load_leafs(bsp);
		}
		else if (i == BSP_LEAFFACES) {
			bsp->data.leaffaces = malloc(len);
			bsp->data.n_leaffaces = len / sizeof(leafface);
			bsp_readentry(fp, entry, bsp->data.leaffaces);
		}
		else if (i == BSP_LEAFBRUSHES) {
			bsp->data.leafbrushes = malloc(len);
			bsp->data.n_leafbrushes = len / sizeof(leafbrush);
			bsp_readentry(fp, entry, bsp->data.leafbrushes);
		}
		else if (i == BSP_NODES) {
			bsp->data.nodes = malloc(len);
			bsp->data.n_nodes = len / sizeof(node);
			bsp_readentry(fp, entry, bsp->data.nodes);
			bsp_load_nodes(bsp);
		}
		else if (i == BSP_PLANES) {
			bsp->data.planes = malloc(len);
			bsp->data.n_planes = len / sizeof(plane);
			bsp_readentry(fp, entry, bsp->data.planes);
			bsp_load_planes(bsp);
		}
		else if (i == BSP_VISDATA) {
			bsp->data.vis = malloc(len);
			bsp_readentry(fp, entry, bsp->data.vis);
			printf("loaded visdata\n");
		}
		else if (i == BSP_BRUSHES) {
			bsp->data.brushes = malloc(len);
			bsp->data.n_brushes = len / sizeof(brush);
			bsp_readentry(fp, entry, bsp->data.brushes);
			printf("%d brushes\n", bsp->data.n_brushes);
		}
		else if (i == BSP_BRUSHSIDES) {
			bsp->data.brushsides = malloc(len);
			bsp->data.n_brushsides = len / sizeof(brushside);
			bsp_readentry(fp, entry, bsp->data.brushsides);
			printf("%d brushsides\n", bsp->data.n_brushsides);
		}
	}
	fclose(fp);

	return bsp;
}

void bsp_free(bspfile *bsp) {
	int i, x;

	free(bsp->data.entities);
	free(bsp->data.textures);
	free(bsp->data.planes);
	free(bsp->data.nodes);
	free(bsp->data.leafs);
	free(bsp->data.leaffaces);
	free(bsp->data.leafbrushes);
	free(bsp->data.models);
	free(bsp->data.brushes);
	free(bsp->data.brushsides);
	free(bsp->data.vertexes);
	free(bsp->data.meshverts);
	free(bsp->data.effects);
	free(bsp->data.faces);
	free(bsp->data.lightmaps);
	free(bsp->data.lightvols);
	free(bsp->data.vis);

	free(bsp->lightmap_indexes);
	free(bsp->texture_indexes);

	for (i = 0; i < bsp->data.n_faces; i++) {
		if (!bsp->tesselpatches[i]) continue;
		for (x = 0; x < bsp->tesselpatches[i]->n_patches; x++) {
			free(bsp->tesselpatches[i]->patches[x].indexes);
			free(bsp->tesselpatches[i]->patches[x].vertexes);
		}
		free(bsp->tesselpatches[i]->patches);
		free(bsp->tesselpatches[i]);
	}
	free(bsp->tesselpatches);

	free(bsp);
	memset(bsp, 0, sizeof(bspfile));
}