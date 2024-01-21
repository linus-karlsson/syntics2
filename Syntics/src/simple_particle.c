#ifndef SY_UNIT_BUILD
#include "simple_particle.h"
#include "region_alloc.h"
#include "random.h"
#include "render_util.h"
#endif

void particles_2d_init(Region_Alloc* region, Particles_2D* particles,
                       u32 max_particles)
{
    particles->curr_index = 0;
    particles->units =
        region_array_calloc(region, max_particles, Particle_Attrib_2D);
    particles->pool_size = max_particles;
}

void particle_2d_emit(Particles_2D* particles,
                      const Particle_Attrib_2D* particle_attrib,
                      V2 individual_speed, V2 neg_alt, f32 life, u32 seed)
{
    Particle_Attrib_2D* curr_particle =
        region_array_value_ptr(particles->units, particles->curr_index);

    *curr_particle = *particle_attrib;
    curr_particle->vel =
        v2_multi(individual_speed,
                 v2_add(v2_neg(neg_alt), v2f(random_f32s(seed++, 0.0f, 1.0f),
                                             random_f32s(seed++, 0.0f, 1.0f))));
    curr_particle->life = v2i(life);

    ++particles->curr_index;
    particles->curr_index %= particles->pool_size;
}

u32 particles_2d_update(Particles_2D* particles, Vertex_Array* vertices, f32 dt)
{
    u32 out = 0;
    for (u32 i = 0; i < particles->pool_size; i++)
    {
        Particle_Attrib_2D* curr_particle =
            region_array_value_ptr(particles->units, i);
        if (curr_particle->life.x > 0.0f)
        {
            curr_particle->position.x += (curr_particle->vel.x * dt);
            curr_particle->position.y += (curr_particle->vel.y * dt);
            curr_particle->position.z = -1.0f;
            curr_particle->life.x -= dt;
            f32 remaining_life = curr_particle->life.x / curr_particle->life.y;
            f32 size = 10.0f * remaining_life;
            quad(vertices, &out, curr_particle->position, v2i(size),
                 curr_particle->color, 2.0f);
        }
    }
    return out;
}

void particles_3d_init(Region_Alloc* region, Particles_3D* particles,
                       u32 max_particles)
{
    particles->curr_index = 0;
    particles->units =
        region_array_calloc(region, max_particles, Particle_Attrib_3D);
    particles->pool_size = max_particles;
}

void particles_3d_reset(Particles_3D* particles)
{
    particles->pool_size = 0;
    particles->curr_index = 0;
}

void particle_3d_emit(Particles_3D* particles,
                      const Particle_Attrib_3D* particle_attrib,
                      V3 individual_speed, V3 neg_alt, f32 random, f32 life)
{
    Particle_Attrib_3D* curr_particle =
        region_array_value_ptr(particles->units, particles->curr_index);

    *curr_particle = *particle_attrib;
    curr_particle->vel =
        v3_multi(individual_speed, v3_s_add(v3_neg(neg_alt), random));
    curr_particle->life = v2i(life);

    ++particles->curr_index;
    particles->curr_index %= particles->pool_size;
}

u32 particles_3d_update(Particles_3D* particles, Vertex_Array* vertices,
                        u32 vertex_offset, f32 dt)
{
    u32 out = 0;
    for (u32 i = 0; i < particles->pool_size; i++)
    {
        Particle_Attrib_3D* curr_particle =
            region_array_value_ptr(particles->units, i);
        if (curr_particle->life.x > 0.0f)
        {
            v3_add_equal(&curr_particle->position,
                         (v3_s_multi(curr_particle->vel, dt)));
            curr_particle->life.x -= dt;
            // const f32 remaining_life = curr_particle->life.x /
            // curr_particle->life.y; const V3 size = v3_lerp(v3d(),
            // curr_particle->size, remaining_life);

            vertex_offset =
                cube(vertices, vertex_offset, curr_particle->position,
                     curr_particle->size, curr_particle->color, 0.0f);
            out++;
        }
    }
    return out;
}
