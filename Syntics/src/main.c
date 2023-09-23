
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
#if 0
    run_app();
#else
    run_notebook_app();
#endif
    
    return 0;
}
#else

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line,
                   int show_cmd)
{
#if 1
    run_app();
#else
    run_notebook_app();
#endif
    return 0;
}
#endif
