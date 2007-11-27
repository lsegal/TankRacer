#include "common.h"
#include "camera.h"

static int Camera_FaceSort(const void *a, const void *b);
static void Camera_GenerateFaceList(Camera *self);

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
	free(self->visitedFaces);
	free(self->faceList);
	free(self);
}

void Camera_Move(Camera *self, float direction[3]) {
	vec3f_add(self->position, direction, self->position);
	Camera_GenerateFaceList(self);
}

void Camera_MoveInDirection(Camera *self, float scale) {
	float dir[] = {
		scale * cosd(self->viewAngles[0]),
		scale * tand(self->viewAngles[1]),
		scale * sind(self->viewAngles[0])
	};
	Camera_Move(self, dir);
}

void Camera_Render(Camera *self) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(self->fov, self->aspectRatio, 0.01, 100000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3d(1, 1, 1);

	gluLookAt(
		self->position[0], 
		self->position[1], 
		self->position[2], 
		self->position[0] + cosd(self->viewAngles[0]), 
		self->position[1] + tand(self->viewAngles[1]), 
		self->position[2] + sind(self->viewAngles[0]), 
		self->upAngles[0],
		self->upAngles[1],
		self->upAngles[2]
	);

	bsp_draw_faces(self->bsp, self->faceList, self->numFaces);
}

static Camera *_camera;
static int Camera_FaceSort(const void *a, const void *b) {
	int i, x, n;
	float dist[2], temp[3];

	/* Get faces from index arguments */
	face *cface[] = { &_camera->bsp->data.faces[*(int*)a], &_camera->bsp->data.faces[*(int*)b] };

	if (_camera->bsp->data.textures[cface[0]->texture].flags & 32) {
		return 1;
	}
	else if (_camera->bsp->data.textures[cface[1]->texture].flags & 32) {
		return -1;
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
	int i, l, x, j;
	leaf *cleaf;
	leafface *lface;

	l = bsp_find_leaf(self->bsp, self->position);

#ifdef _PRINTDEBUG
	printf("Leaf number %d, visdata: %d, leaffaces: %d\n", l, bsp->data.leafs[l].cluster, bsp->data.leafs[l].n_leaffaces);
#endif

	if (l >= 0 && self->bsp->data.leafs[l].cluster >= 0) {
		self->numFaces = 0;
		memset(self->visitedFaces, 0, sizeof(int) * self->bsp->data.n_faces);

		cleaf = &self->bsp->data.leafs[l];

		for (x = 0; x < self->bsp->data.n_leafs; x++) {
			if (x != l && !bsp_leaf_visible(self->bsp, cleaf, &self->bsp->data.leafs[x])) {
				continue;
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
