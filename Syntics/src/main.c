
/*
internal f32 abs_f32(f32 val)
{
    *((u32*)&val) &= 0x7FFFFFFF;
    return val;
}

internal i32 abs_i32(i32 val)
{
    i32 mask = val >> 31;
    i32 res = (val + mask) ^ mask;
    return res;
}

*/

#ifdef LINUX
int main(int argc, char* argv[])
{
    run_app();
    return 0;
}
#else


int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line,
                   int show_cmd)
{
    run_app();
    return 0;
}
#endif
