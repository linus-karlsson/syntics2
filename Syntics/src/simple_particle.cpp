#include "simple_particle.h"
#include "region_alloc.h"
#include "random.h"
#include "vulkan_types.h"
#include "buffers.h"

void init_particles(Region_Alloc* region, Particles& particles, u32 max_particles)
{
    particles.curr_index = 0;
    particles.units = dyn_arrayP(region, max_particles, Particle_Attrib);
    particles.pool_size = max_particles;

    memset(particles.units, 0, max_particles * sizeof(Particle_Attrib));
}

void emit_particle(Particles& particles, const Particle_Attrib& particle_attrib,
                   const Vec2& individual_speed, const Vec2& neg_alt, f32 life)
{
    Particle_Attrib* curr_particle = &particles.units[particles.curr_index];

    *curr_particle = particle_attrib;
    curr_particle->vel.x = (individual_speed.x * (rand_f32(0.0f, 1.0f) - neg_alt.x));
    curr_particle->vel.y = (individual_speed.y * (rand_f32(0.0f, 1.0f) - neg_alt.y));
    curr_particle->life = Vec2(life);

    ++particles.curr_index %= particles.pool_size;
}

u32 update_particles(Particles& particles, Vertex** vertices, f32 dt)
{
    u32 out = 0;
    for (u32 i = 0; i < particles.pool_size; i++)
    {
        Particle_Attrib* curr_particle = &particles.units[i];
        if (curr_particle->life.x > 0.0f)
        {
            curr_particle->position.x += (curr_particle->vel.x * dt);
            curr_particle->position.y += (curr_particle->vel.y * dt);
            curr_particle->position.z = -1.0f;
            curr_particle->life.x -= dt;
            f32 remaining_life = curr_particle->life.x / curr_particle->life.y;
            f32 size = 10.0f * remaining_life;
            quad(vertices, &out, curr_particle->position, Vec2(size),
                 curr_particle->color, 2.0f);
        }
    }
    return out;
}

