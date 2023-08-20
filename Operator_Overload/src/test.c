
f32 movement_speed = dude.misc->speed;
dude.movement->acc = v3d();
{
    if (is_key_pressed(SYNT_KEY_SHIFT))
    {
        movement_speed *= speed_multiplier_GAME;
    }
    if (is_key_pressed(SYNT_KEY_W))
    {
        dude.movement->acc = v3_add(dude.movement->acc, v3_s_multi(dude_ori, movement_speed));
    }
    if (is_key_pressed(SYNT_KEY_S))
    {
        dude.movement->acc = v3_add(dude.movement->acc, v3_s_multi(dude_ori, -movement_speed));
    }
    if (is_key_pressed(SYNT_KEY_A))
    {
        dude.animation->angle += rotation_speed * dt;
    }
    if (is_key_pressed(SYNT_KEY_D))
    {
        dude.animation->angle += -rotation_speed * dt;
    }
    if (is_key_pressed(SYNT_KEY_Q))
    {
        V3 side_vector = v3_normalize(v3_cross(dude_ori, game->cam.up));
        dude.movement->acc = v3_add(dude.movement->acc, v3_s_multi(side_vector, -movement_speed));
    }
}
