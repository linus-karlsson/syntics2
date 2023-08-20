void update()
{
    V3 thing = v3f(1.0f, 3.0f, 4.0f);
    V3 dd = v3d();

    dd = thing v * dd v - thing;
}
