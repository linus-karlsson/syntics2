#pragma once

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
