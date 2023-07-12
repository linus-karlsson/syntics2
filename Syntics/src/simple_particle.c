
void init_particles_2d(Region_Alloc* region, Particles_2D* particles,
                       u32 max_particles)
{
    particles->curr_index = 0;
    particles->units =
        region_array_callocP(region, max_particles, Particle_Attrib_2D);
    particles->pool_size = max_particles;
}

void emit_particle_2d(Particles_2D* particles,
                      const Particle_Attrib_2D* particle_attrib, V2 individual_speed,
                      V2 neg_alt, f32 life)
{
    Particle_Attrib_2D* curr_particle =
        array_val_ptr(particles->units, particles->curr_index);

    *curr_particle = *particle_attrib;
    curr_particle->vel = v2_multi(
        individual_speed,
        v2_add(v2_neg(neg_alt), v2f(rand_f32(0.0f, 1.0f), rand_f32(0.0f, 1.0f))));
    curr_particle->life = v2i(life);

    ++particles->curr_index;
    particles->curr_index %= particles->pool_size;
}

u32 update_particles_2d(Particles_2D* particles, Vertex* vertices, f32 dt)
{
    u32 out = 0;
    for (u32 i = 0; i < particles->pool_size; i++)
    {
        Particle_Attrib_2D* curr_particle = array_val_ptr(particles->units, i);
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

void init_particles_3d(Region_Alloc* region, Particles_3D* particles,
                       u32 max_particles)
{
    particles->curr_index = 0;
    particles->units =
        region_array_callocP(region, max_particles, Particle_Attrib_3D);
    particles->pool_size = max_particles;
}

void emit_particle_3d(Particles_3D* particles,
                      const Particle_Attrib_3D* particle_attrib, V3 individual_speed,
                      V3 neg_alt, f32 random, f32 life)
{
    Particle_Attrib_3D* curr_particle =
        array_val_ptr(particles->units, particles->curr_index);

    *curr_particle = *particle_attrib;
    curr_particle->vel =
        v3_multi(individual_speed, v3_s_add(v3_neg(neg_alt), random));
    curr_particle->life = v2i(life);

    ++particles->curr_index;
    particles->curr_index %= particles->pool_size;
}

u32 update_particles_3d(Particles_3D* particles, Vertex* vertices, u32 vertex_offset,
                        f32 dt)
{
    u32 out = 0;
    for (u32 i = 0; i < particles->pool_size; i++)
    {
        Particle_Attrib_3D* curr_particle = array_val_ptr(particles->units, i);
        if (curr_particle->life.x > 0.0f)
        {
            v3_add_equal(&curr_particle->position,
                         (v3_s_multi(curr_particle->vel, dt)));
            curr_particle->life.x -= dt;
            // f32 remaining_life = curr_particle->life.x / curr_particle->life.y;
            // f32 size = 10.0f * remaining_life;

            vertex_offset = cube(vertices, vertex_offset, curr_particle->position,
                                 curr_particle->size, curr_particle->color, 0.0f);
            out++;
        }
    }
    return out;
}

