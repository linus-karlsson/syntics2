#pragma once

#include "defines.h"
#include "math/vectors.h"

namespace synt {

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
    uint32 pool_size;
    uint32 curr_index;
} Particles;

typedef struct Region_Alloc Region_Alloc;
typedef struct Vertex Vertex;

void init_particles(Region_Alloc* region, Particles& particles,
                    uint32 max_particles);

void emit_particle(Particles& particles, const Particle_Attrib& particle_attrib,
                   const Vec2& individual_speed, const Vec2& neg_alt, float life);

uint32 update_particles(Particles& particles, Vertex** vertices, float dt);

} // namespace synt
