#pragma once

#include "defines.h"
#include "math/vectors.h"

typedef struct Particle_Attrib_2D
{
    V3 position;
    V4 color;
    V2 vel;
    V2 size;
    V2 life;
} Particle_Attrib_2D;

typedef struct Particles_2D
{
    Particle_Attrib_2D* units;
    u32 pool_size;
    u32 curr_index;
} Particles_2D;

typedef struct Particle_Attrib_3D
{
    V3 position;
    V4 color;
    V3 vel;
    V3 size;
    V2 life;
} Particle_Attrib_3D;

typedef struct Particles_3D
{
    Particle_Attrib_3D* units;
    u32 pool_size;
    u32 curr_index;
} Particles_3D;

void init_particles(Region_Alloc* region, Particles_2D& particles,
                    u32 max_particles);

void emit_particle(Particles_2D& particles,
                   const Particle_Attrib_2D& particle_attrib, V2 individual_speed,
                   V2 neg_alt, f32 life);

u32 update_particles(Particles_2D& particles, Vertex* vertices, f32 dt);

void init_particles(Region_Alloc* region, Particles_3D& particles,
                    u32 max_particles);

void emit_particle(Particles_3D& particles,
                   const Particle_Attrib_3D& particle_attrib, V3 individual_speed,
                   V3 neg_alt, f32 random, f32 life);

u32 update_particles(Particles_3D& particles, Vertex* vertices, u32 vertex_offset,
                     f32 dt);

