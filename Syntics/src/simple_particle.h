#pragma once

#include "defines.h"
#include "math/vectors.h"

typedef struct Particle_Attrib
{
    V3 position;
    V4 color;
    V2 vel;
    V2 size;
    V2 life;
} Particle_Attrib;

typedef struct Particles
{
    Particle_Attrib* units;
    u32 pool_size;
    u32 curr_index;
} Particles;

typedef struct Region_Alloc Region_Alloc;
typedef struct Vertex Vertex;

void init_particles(Region_Alloc* region, Particles& particles, u32 max_particles);

void emit_particle(Particles& particles, const Particle_Attrib& particle_attrib,
                   const V2& individual_speed, const V2& neg_alt, f32 life);

u32 update_particles(Particles& particles, Vertex** vertices, f32 dt);

