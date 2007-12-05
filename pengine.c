#include "common.h"
#include "pengine.h"

static void Particle_Spawn(ParticleEngine *pengine, Particle *particle) {
	int i;

	/* Set position */
	vec3f_set(pengine->startPosition, particle->position);

	/* Randomize life */
	particle->life = pengine->startLife - (rand() % pengine->spray * 3);

	/* Randomize direction */
	for (i = 0; i < 3; i++) {
		particle->force[i] = pengine->startForce[i] + 
			(double)((rand() % pengine->spray) - 
			((double)pengine->spray/50)) / ((double)pengine->spray * 20);
	}
}

ParticleEngine *ParticleEngine_Init(int size, int life, int spray, float floor, GLuint texture,
									float position[3], float force[3], float gravity[3] ) {
	ParticleEngine *p = malloc(sizeof(ParticleEngine) + sizeof(Particle) * size);
	memset(p, 0, sizeof(ParticleEngine));

	if (position) vec3f_set(position, p->startPosition);
	else vec3f_clear(p->startPosition);
	if (force) vec3f_set(force, p->startForce);
	else vec3f_clear(p->startForce);

	p->particles = malloc(sizeof(Particle) * size);
	memset(p->particles, 0, sizeof(Particle) * size);

	p->size = size;
	p->floor = floor;
	p->startLife = life;
	p->spray = spray;
	p->texture = texture;

	return p;
}

void ParticleEngine_Free(ParticleEngine *pengine) {
	free(pengine);
}

void ParticleEngine_Run(ParticleEngine *pengine) {
	int i;
	Particle *particle;

	for (i = 0; i < pengine->size; i++) {
		particle = &pengine->particles[i];

		/* Respawn particle if dead */
		if (particle->life-- <= 0) {
			Particle_Spawn(pengine, particle);
		}

		/* Add force to position */
		vec3f_add(particle->position, particle->force, particle->position);

		/* Don't go through floor */
		if (particle->position[1] < pengine->floor) {
			particle->position[1] = pengine->floor;
		}

		/* Render particle */
		glColor3f(1,1,0);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
		glBindTexture(GL_TEXTURE_2D, pengine->texture);
		glBegin(GL_QUADS);
		glTexCoord2d(0, 0);
		glVertex3f(particle->position[0] - 0.1, particle->position[1], particle->position[2] - 0.1);
		glTexCoord2d(0, 1);
		glVertex3f(particle->position[0] - 0.1, particle->position[1], particle->position[2] + 0.1);
		glTexCoord2d(1, 1);
		glVertex3f(particle->position[0] + 0.1, particle->position[1], particle->position[2] + 0.1);
		glTexCoord2d(1, 0);
		glVertex3f(particle->position[0] + 0.1, particle->position[1], particle->position[2] - 0.1);
		glEnd();
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);

		/* Add gravity to force */
		vec3f_add(particle->force, pengine->gravity, particle->force);
	}
}