
f32 movement_speed = dude.misc->speed;
dude.movement->acc = v3d();
{
    if (is_key_pressed(SYNT_KEY_SHIFT))
    {
        movement_speed *= speed_multiplier_GAME;
    }
    if (is_key_pressed(SYNT_KEY_W))
    {
        // OPER v3 v3 s
        dude.movement->acc = dude.movement->acc + dude_ori * movement_speed;
    }
    if (is_key_pressed(SYNT_KEY_S))
    {
        // OPER v3 v3 s
        dude.movement->acc = dude.movement->acc + dude_ori * -movement_speed;
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
        // OPER v3 v3 s
        dude.movement->acc = dude.movement->acc + side_vector * -movement_speed;
    }
}
