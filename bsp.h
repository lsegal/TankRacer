#ifndef HAVE_BSP_H
#define HAVE_BSP_H

#define BSP_SCALE 64

const enum direntrynames {
	BSP_ENTITIES,
	BSP_TEXTURES,
	BSP_PLANES,
	BSP_NODES,
	BSP_LEAFS,
	BSP_LEAFFACES,
	BSP_LEAFBRUSHES,
	BSP_MODELS,
	BSP_BRUSHES,
	BSP_BRUSHSIDES,
	BSP_VERTEXES,
	BSP_MESHVERTS,
	BSP_EFFECTS,
	BSP_FACES,
	BSP_LIGHTMAPS,
	BSP_LIGHTVOLS,
	BSP_VISDATA,
	BSP_NUM_LUMPS
} dirtentrynames;

const enum facetypes {
	FACE_POLYGON = 1,
	FACE_PATCH,
	FACE_MESH,
	FACE_BILLBOARD
} facetypes;

typedef struct direntry {
	int offset;
	int length;
} direntry;

typedef struct entity {
	ubyte	*ents;			/* Entities */
} entity;

typedef struct texture {
	ubyte	name[64];		/* Texture name  */
	int		flags;			/* Surface flags */
	int		contents;		/* Content flags */
} texture;

typedef struct plane {
	float	normal[3];		/* Plane normal */
	float	dist;			/* Distance from origin to plane along normal */
} plane;

typedef struct node {
	int		plane;			/* Plane index */
	int		front;			/* Children indices. Negative numbers are leaf indices: -(leaf+1). */
	int		back;			/* Children indices. Negative numbers are leaf indices: -(leaf+1). */
	int		mins[3];		/* Integer bounding box min coord. */
	int		maxs[3];		/* Integer bounding box max coord. */
} node;

typedef struct leaf {
	int		cluster;		/* Visdata cluster index.			*/
	int		area;			/* Areaportal area.					*/
	int		mins[3];		/* Integer bounding box min coord.  */
	int		maxs[3];		/* Integer bounding box max coord.  */
	int		leafface;		/* First leafface for leaf.			*/
	int		n_leaffaces;	/* Number of leaffaces for leaf.	*/
	int		leafbrush;		/* First leafbrush for leaf.		*/
	int		n_leafbrushes;	/* Number of leafbrushes for leaf.  */
} leaf;

typedef struct leafface {
	int		face;			/* Face index */
} leafface;

typedef struct leafbrush {
	int		brush;			/* Brush index */
} leafbrush;

typedef struct model {
	float	mins[3];		/* Bounding box min coord.		 */
	float	maxs[3];		/* Bounding box max coord.		 */
	int		face;			/* First face for model.		 */
	int		n_faces;		/* Number of faces for model.	 */
	int		brush;			/* First brush for model.		 */
	int		n_brushes;		/* Number of brushes for model.	 */
} model;

typedef struct brush {
	int		brushside;		/* First brushside for brush.		*/
	int		n_brushsides;	/* Number of brushsides for brush	*/
	int		texture;		/* Texture index					*/
} brush;

typedef struct brushside {
	int		plane;			/* Plane index		*/
	int		texture;		/* Texture index	*/
} brushside;

typedef struct vertex {
	float	position[3];	/* Vertex position								*/
	float	texcoord[2][2];	/* Texture coordinate. 0=surface, 1=lightmap.	*/
	float	normal[3];		/* Vertex normal								*/
	ubyte	color[4];		/* Vertex color (RGBA)							*/
} vertex;

typedef struct meshvert {
	int		offset;			/* Vertex index offset relative to first vertex of face */
} meshvert;

typedef struct effect {
	ubyte	name[64];		/* Effect shader					*/
	int		brush;			/* Brush that generated the effect	*/
	int		unknown;		/* Always 5, except in q3dm8 (-1)	*/
} effect;

typedef struct face {
	int		texture;		/* Texture index.									   */
	int		effect;			/* Index into lump 12 (Effects), or -1.				   */
	int		type;			/* Face type. 1=polygon, 2=patch, 3=mesh, 4=billboard  */
	int		vertex;			/* Index of first vertex.							   */
	int		n_vertexes;		/* Number of vertices.								   */
	int		meshvert;		/* Index of first meshvert.							   */
	int		n_meshverts;	/* Number of meshverts.								   */
	int		lm_index;		/* Lightmap index.									   */
	int		lm_start[2];	/* Corner of this face's lightmap image in lightmap.   */
	int		lm_size[2];		/* Size of this face's lightmap image in lightmap.     */
	float	lm_origin[3];	/* World space origin of lightmap.					   */
	float	lm_vecs[2][3];	/* World space lightmap s and t unit vectors.		   */
	float	normal[3];		/* Surface normal.									   */
	int		size[2];		/* Patch dimensions.								   */
} face;

typedef struct lightmap {
	ubyte	map[128][128][3]; /* Lightmap color data (RGB) */
} lightmap;

typedef struct lightvol {
	ubyte	ambient[3];		/* Ambient color component (RGB)		*/
	ubyte	directional[3]; /* Directional color component (RGB)	*/
	ubyte	dir[2];			/* Direction to light. 0=phi, 1=theta	*/
} lightvol;

typedef struct visdata {
	int		n_vecs;			/* Number of vectors							*/
	int		sz_vecs;		/* Size of each vector in bytes					*/
	ubyte	vecs[1];		/* Visibility data (n_vecs * sz_vecs in size)	*/
} visdata;

typedef struct bspheader {
	ubyte		magic[4];	/* Always "IBSP" */
	int			version;	
	direntry	direntries[17];
} bspheader;

typedef struct bspdata {
	entity		*entities;
	texture		*textures;
	plane		*planes;
	node		*nodes;
	leaf		*leafs;
	leafface	*leaffaces;
	leafbrush	*leafbrushes;
	model		*models;
	brush		*brushes;
	brushside	*brushsides;
	vertex		*vertexes;
	meshvert	*meshverts;
	effect		*effects;
	face		*faces;
	lightmap	*lightmaps;
	lightvol	*lightvols;
	visdata		*vis;

	int			n_entities;
	int			n_textures;
	int			n_planes;
	int			n_nodes;
	int			n_leafs;
	int			n_leaffaces;
	int			n_leafbrushes;
	int			n_models;
	int			n_brushes;
	int			n_brushsides;
	int			n_vertexes;
	int			n_meshverts;
	int			n_effects;
	int			n_faces;
	int			n_lightmaps;
	int			n_lightvols;
} bspdata;

typedef struct tesselpatch {
	int			tesslevel;
	vertex		ctlpoints[9];
	vertex		*vertexes;
	int			*indexes;
} tesselpatch;

typedef struct patchlist {
	int			n_patches;
	tesselpatch *patches;
} patchlist;

typedef struct bspfile {
	bspheader	header;
	bspdata		data;
	int			*texture_indexes;
	int			*lightmap_indexes;
	patchlist   **tesselpatches;
} bspfile;

extern bspfile  *bsp_load(char *);
extern void	 	 bsp_free(bspfile *);
extern void      bsp_draw_faces(bspfile *, int *, int);
extern leaf	    *bsp_find_leaf(bspfile *, float[3]);
extern int	 	 bsp_leaf_visible(bspfile *, leaf *, leaf *);
extern plane    *bsp_simple_collision(bspfile *, float[3], float[3], leaf *, leaf *);
extern face	    *bsp_face_collision(bspfile *, float[3], float[3], int);
extern lightvol *bsp_lightvol(bspfile *, float[3]);

#endif /* HAVE_BSP_H */