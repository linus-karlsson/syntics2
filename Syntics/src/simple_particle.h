#pragma once

#include "defines.h"
#include "math/vectors.h"

typedef struct Particle_Attrib
{
    Vec3 position;
    Vec4 color;
    Vec2 vel;
    Vec2 size;
    Vec2 life;
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
                   const Vec2& individual_speed, const Vec2& neg_alt, f32 life);

u32 update_particles(Particles& particles, Vertex** vertices, f32 dt);

