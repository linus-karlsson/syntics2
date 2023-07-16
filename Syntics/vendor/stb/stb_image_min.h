enum
{
    STBI_default = 0, // only used for desired_channels
    STBI_grey = 1,
    STBI_grey_alpha = 2,
    STBI_rgb = 3,
    STBI_rgb_alpha = 4
};
unsigned char* stbi_load(char const* filename, int* x, int* y, int* comp,
                         int req_comp);
