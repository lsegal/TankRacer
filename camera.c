#include "common.h"
#include "camera.h"

static const enum BoxSides { 
	SIDE_RIGHT, SIDE_LEFT, SIDE_BOTTOM, SIDE_TOP, SIDE_FAR, SIDE_NEAR 
} BoxSides;

static int  Camera_FaceSort(const void *a, const void *b);
static void Camera_GenerateFaceList(Camera *self);
static int  Camera_FrustumCull(Camera *self, leaf *testLeaf);
static void Camera_SetFrustum(Camera *self);

void Camera_Init(Camera *self, bspfile *bsp) {
	int size;

	memset(self, 0, sizeof(Camera));

	self->bsp = bsp;
	self->fov = 60;
	self->aspectRatio = 1;

	/* Setup renderer */
	size = sizeof(int) * bsp->data.n_faces;
	self->visitedFaces = malloc(size);
	self->faceList     = malloc(size);
	self->numFaces	   = 0;

	/* Up angles */
	self->upAngles[1] = 1;
}

void Camera_Free(Camera *self) {
	if (!self) return;
	free(self->visitedFaces);
	free(self->faceList);
	free(self);
}

void Camera_SetPosition(Camera *self, float position[3], float direction[3]) {
	vec3f_set(position, self->position);
	vec3f_set(direction, self->direction);
	Camera_GenerateFaceList(self);
}

void Camera_Move(Camera *self, float direction[3]) {
	int i;
	float temp[3], dist, t, m;
	plane *cplane;

	vec3f_add(self->position, direction, temp);

	cplane = bsp_simple_collision(self->bsp, self->position, temp, self->currentLeaf, NULL);
	if (cplane && self->currentLeaf && self->currentLeaf->cluster >= 0) {
		m = vec3f_mag(direction);
		vec3f_set(direction, temp);
		vec3f_norm(temp);
		for (i = 0; i < 3; i++) {
			t = temp[i];
			temp[i] += cplane->normal[i];
			if (temp[i] >= 0 && t < 0 || t >= 0 && temp[i] < 0) {
				temp[i] = 0;
			}
		}
		vec3f_scale(temp, m, temp);
		vec3f_add(self->position, temp, temp);
	}
	
	vec3f_set(temp, self->position);
	Camera_GenerateFaceList(self);
}

void Camera_MoveInDirection(Camera *self, float scale) {
	float dir[] = {
		scale * self->direction[0],
		scale * self->direction[1],
		scale * self->direction[2]
	};
	Camera_Move(self, dir);
}

void Camera_Render(Camera *self) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(self->fov, self->aspectRatio, 0.01, 100000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(
		self->position[0], 
		self->position[1], 
		self->position[2], 
		self->position[0] + self->direction[0], 
		self->position[1] + self->direction[1], 
		self->position[2] + self->direction[2], 
		self->upAngles[0],
		self->upAngles[1],
		self->upAngles[2]
	);

//	Camera_SetFrustum(self);

	bsp_draw_faces(self->bsp, self->faceList, self->numFaces);
}

static void Camera_SetFrustum(Camera *self) {
	int i;
	float mag, projection[16], view[16], clip[16];

	glGetFloatv(GL_PROJECTION_MATRIX, projection);
	glGetFloatv(GL_MODELVIEW_MATRIX, view);

	mat4f_mult(projection, view, clip);

	self->frustum[SIDE_RIGHT][0] = clip[3] - clip[0];
	self->frustum[SIDE_RIGHT][1] = clip[7] - clip[4];
	self->frustum[SIDE_RIGHT][2] = clip[11] - clip[8];
	self->frustum[SIDE_RIGHT][3] = clip[15] - clip[12];

	self->frustum[SIDE_LEFT][0] = clip[3] + clip[0];
	self->frustum[SIDE_LEFT][1] = clip[7] + clip[4];
	self->frustum[SIDE_LEFT][2] = clip[11] + clip[8];
	self->frustum[SIDE_LEFT][3] = clip[15] + clip[12];

	self->frustum[SIDE_BOTTOM][0] = clip[3] + clip[1];
	self->frustum[SIDE_BOTTOM][1] = clip[7] + clip[5];
	self->frustum[SIDE_BOTTOM][2] = clip[11] + clip[9];
	self->frustum[SIDE_BOTTOM][3] = clip[15] + clip[13];

	self->frustum[SIDE_TOP][0] = clip[3] - clip[1];
	self->frustum[SIDE_TOP][1] = clip[7] - clip[5];
	self->frustum[SIDE_TOP][2] = clip[11] - clip[9];
	self->frustum[SIDE_TOP][3] = clip[15] - clip[13];

	self->frustum[SIDE_FAR][0] = clip[3] - clip[2];
	self->frustum[SIDE_FAR][1] = clip[7] - clip[6];
	self->frustum[SIDE_FAR][2] = clip[11] - clip[10];
	self->frustum[SIDE_FAR][3] = clip[15] - clip[14];

	self->frustum[SIDE_NEAR][0] = clip[3] + clip[2];
	self->frustum[SIDE_NEAR][1] = clip[7] + clip[6];
	self->frustum[SIDE_NEAR][2] = clip[11] + clip[10];
	self->frustum[SIDE_NEAR][3] = clip[15] + clip[14];

	for (i = 0; i < 6; i++) {
		mag = vec3f_mag(self->frustum[i]);
		self->frustum[i][0] /= mag;
		self->frustum[i][1] /= mag;
		self->frustum[i][2] /= mag;
		self->frustum[i][3] /= mag;
	}
}

static int Camera_FrustumCull(Camera *self, leaf *testLeaf) {
	float dist, temp[3];
	int i;

	for (i = 0; i < 6; i++) {
		temp[0] = (float)testLeaf->mins[0];
		temp[1] = (float)testLeaf->mins[1];
		temp[2] = (float)testLeaf->mins[2];
		dist = vec3f_dot(temp, self->frustum[i]) - self->frustum[i][3];
		if (dist >= -EPSILON) continue;

		temp[0] = (float)testLeaf->maxs[0];
		temp[1] = (float)testLeaf->maxs[1];
		temp[2] = (float)testLeaf->maxs[2];
		dist = vec3f_dot(temp, self->frustum[i]) - self->frustum[i][3];
		if (dist >= -EPSILON) continue;

		return 0;
	}
	return 1;
}

static Camera *_camera;
static int Camera_FaceSort(const void *a, const void *b) {
	int i, x, n;
	float dist[2], temp[3];

	/* Get faces from index arguments */
	face *cface[] = { &_camera->bsp->data.faces[*(int*)a], &_camera->bsp->data.faces[*(int*)b] };

	if (_camera->bsp->data.textures[cface[0]->texture].flags & 32) {
		return -1;
	}
	else if (_camera->bsp->data.textures[cface[1]->texture].flags & 32) {
		return 1;
	}

	for (x = 0; x < 2; x++) {
/*
		memset(&temp, 0, sizeof(float[3]));

		for (i = 0; i < cface[x]->n_vertexes; i++) {
			for (n = 0; n < 3; n++) {
				temp[n] += _bsp->data.vertexes[cface[x]->vertex].position[n];
			}
		}
		for (n = 0; n < 3; n++) temp[n] /= cface[x]->n_vertexes;
*/
		dist[x] = vec3f_dist(_camera->position, _camera->bsp->data.vertexes[cface[x]->vertex].position);
	}

	return dist[0] < dist[1] ? -1 : 1;
}

static void Camera_GenerateFaceList(Camera *self) {
	int i, x, j;
	leafface *lface;

	self->currentLeaf = bsp_find_leaf(self->bsp, self->position);

#ifdef _PRINTDEBUG
	printf("Leaf number %d, visdata: %d, leaffaces: %d\n", cleaf - bsp->data.leafs, cleaf->cluster, cleaf->n_leaffaces);
#endif

	if (self->currentLeaf->cluster >= 0) {
		self->numFaces = 0;
		memset(self->visitedFaces, 0, sizeof(int) * self->bsp->data.n_faces);

		for (x = 0; x < self->bsp->data.n_leafs; x++) {
			if (&self->bsp->data.leafs[x] != self->currentLeaf) { 
				/* These checks only apply if this isn't the camera leaf */
				if (!bsp_leaf_visible(self->bsp, self->currentLeaf, &self->bsp->data.leafs[x])) 
					continue;

				//if (!Camera_FrustumCull(self, &self->bsp->data.leafs[x]))
				//	continue;
			}

			for (j = 0; j < self->bsp->data.leafs[x].n_leaffaces; j++) {
				lface = &self->bsp->data.leaffaces[self->bsp->data.leafs[x].leafface+j];

				if (self->visitedFaces[lface->face]) continue;
				self->faceList[self->numFaces++] = lface->face;
				self->visitedFaces[lface->face] = 1;
			}
		}
	}
	else { /* Outside the map, draw all faces */
		self->numFaces = self->bsp->data.n_faces;
		for (i = 0; i < self->numFaces; i++) {
			self->faceList[i] = i;
		}
	}

	/* Sort the faces from farthest to closest */
	if (self->numFaces < 500) {
		_camera = self; /* EW! */
		qsort(self->faceList, self->numFaces, sizeof(int), Camera_FaceSort);
	}
}
