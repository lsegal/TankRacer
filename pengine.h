typedef struct Particle {
	int   life;			/* Number of remaining frames that the particle lives for */
	float position[3];	/* Position of particle */
	float force[3];		/* Direction vector, essentially */
} Particle;

typedef struct ParticleEngine {
	int   size;				/* Number of particles */
	int   spray;			/* Random spray direction */
	int   startLife;		/* Max life */
	float floor;			/* Absolute minimum Y value for position */		
	GLuint texture;			/* Particle texture */
	float gravity[3];		/* Gravity vector */
	float startPosition[3];	/* Start position */
	float startForce[3];	/* Start force */
	Particle *particles;	/* Particles list */
} ParticleEngine;

extern ParticleEngine *ParticleEngine_Init(int size, int life, int spray, float floor, GLuint texture,
									float position[3], float force[3], float gravity[3]);
extern void ParticleEngine_Free(ParticleEngine *pengine);
extern void ParticleEngine_Run(ParticleEngine *pengine);
