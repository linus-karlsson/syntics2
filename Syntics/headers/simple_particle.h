#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#include "math/syntics_math.h"
#endif

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

void particles_2d_init(Region_Alloc* region, Particles_2D* particles,
                       u32 max_particles);
void particle_2d_emit(Particles_2D* particles,
                      const Particle_Attrib_2D* particle_attrib,
                      V2 individual_speed, V2 neg_alt, f32 life, u32 seed);
u32 particles_2d_update(Particles_2D* particles, Vertex_Array* vertices, f32 dt);
void particles_3d_init(Region_Alloc* region, Particles_3D* particles,
                       u32 max_particles);
void particles_3d_reset(Particles_3D* particles);
void particle_3d_emit(Particles_3D* particles,
                      const Particle_Attrib_3D* particle_attrib,
                      V3 individual_speed, V3 neg_alt, f32 random, f32 life);
u32 particles_3d_update(Particles_3D* particles, Vertex_Array* vertices,
                        u32 vertex_offset, f32 dt);
