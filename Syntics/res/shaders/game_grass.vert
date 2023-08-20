#version 450

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec2 i_tex_coords;
layout(location = 3) in vec4 i_color;
layout(location = 4) in float i_tex_index;

layout(location = 0) out vec4 f_color;
layout(location = 1) out vec2 f_tex_coord;
layout(location = 2) out flat float f_tex_index;
layout(location = 3) out vec3 f_normal;

layout(binding = 0) uniform ViewProjection
{
    mat4 view;
    mat4 proj;
}
VP;

layout(push_constant) uniform PushConstant
{
    mat4 model;
    mat4 normal;
}
Push;

int PERMUTATION[] = {
    151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233, 7,   225,
    140, 36,  103, 30,  69,  142, 8,   99,  37,  240, 21,  10,  23,  190, 6,   148,
    247, 120, 234, 75,  0,   26,  197, 62,  94,  252, 219, 203, 117, 35,  11,  32,
    57,  177, 33,  88,  237, 149, 56,  87,  174, 20,  125, 136, 171, 168, 68,  175,
    74,  165, 71,  134, 139, 48,  27,  166, 77,  146, 158, 231, 83,  111, 229, 122,
    60,  211, 133, 230, 220, 105, 92,  41,  55,  46,  245, 40,  244, 102, 143, 54,
    65,  25,  63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,  18,  169,
    200, 196, 135, 130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186, 3,   64,
    52,  217, 226, 250, 124, 123, 5,   202, 38,  147, 118, 126, 255, 82,  85,  212,
    207, 206, 59,  227, 47,  16,  58,  17,  182, 189, 28,  42,  223, 183, 170, 213,
    119, 248, 152, 2,   44,  154, 163, 70,  221, 153, 101, 155, 167, 43,  172, 9,
    129, 22,  39,  253, 19,  98,  108, 110, 79,  113, 224, 232, 178, 185, 112, 104,
    218, 246, 97,  228, 251, 34,  242, 193, 238, 210, 144, 12,  191, 179, 162, 241,
    81,  51,  145, 235, 249, 14,  239, 107, 49,  192, 214, 31,  181, 199, 106, 157,
    184, 84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236, 205, 93,
    222, 114, 67,  29,  24,  72,  243, 141, 128, 195, 78,  66,  215, 61,  156, 180
};

int sy_noise2(int x, int y)
{
    int tmp = PERMUTATION[(y + 0) % 256];
    return PERMUTATION[(tmp + x) % 256];
}

float sy_lerp(float a, float b, float t)
{
    return a + (t * (b - a));
}

float sy_smooth_inter(float a, float b, float t)
{
    return sy_lerp(a, b, t * t * (3 - 2 * t));
}

float sy_noise2d(float x, float y)
{
    int x_int = int(x);
    int y_int = int(y);
    float x_frac = x - x_int;
    float y_frac = y - y_int;
    int s = sy_noise2(x_int, y_int);
    int t = sy_noise2(x_int + 1, y_int);
    int u = sy_noise2(x_int, y_int + 1);
    int v = sy_noise2(x_int + 1, y_int + 1);
    float low = sy_smooth_inter(float(s), float(t), x_frac);
    float high = sy_smooth_inter(float(u), float(v), x_frac);
    return sy_smooth_inter(low, high, y_frac);
}

float sy_value_noise2d(float x, float y, float freq, float gain, int oct)
{
    float amp = gain;
    float result = 0.0;
    float max_ = 0.0;

    for (int i = 0; i < oct; i++)
    {
        max_ += 256.0 * amp;
        result += sy_noise2d(x * freq, y * freq) * amp;
        amp *= gain;
        freq *= 2.0;
    }
    return result / max_;
}

mat4 rotate_x(float rad)
{
    mat4 res = mat4(1.0);
    res[1][1] = cos(rad);
    res[1][2] = sin(rad);
    res[2][1] = -sin(rad);
    res[2][2] = cos(rad);
    return res;
}

mat4 rotate_y(float rad)
{
    mat4 res = mat4(1.0);
    res[0][0] = cos(rad);
    res[0][2] = -sin(rad);
    res[2][0] = sin(rad);
    res[2][2] = cos(rad);
    return res;
}

mat4 rotate_z(float rad)
{
    mat4 res = mat4(1.0);
    res[0][0] = cos(rad);
    res[0][1] = sin(rad);
    res[1][0] = -sin(rad);
    res[1][1] = cos(rad);
    return res;
}

mat4 m4_shear(vec2 hx, vec2 hy, vec2 hz)
{
    mat4 res = mat4(1.0);

    res[0][1] = hy.x;
    res[0][2] = hz.x;
    res[1][0] = hx.x;

    res[1][2] = hz.y;
    res[2][0] = hx.y;
    res[2][1] = hy.y;

    return res;
}

float rand(vec2 v)
{
    return fract(sin(dot(v, vec2(12.9898, 78.233))) * 43758.5453);
}

bool collides(vec2 point, vec2 target_min, vec2 target_size)
{
    bool result = point.x >= target_min.x && point.y >= target_min.y &&
                  point.x < target_min.x + target_size.x &&
                  point.y < target_min.y + target_size.y;
    return result;
}

void main()
{
    float freq = 0.7 * i_tex_index * 0.1;
    float grain = 0.8;
    int oct = 2;
    float offset_increase = 0.1;

    vec3 offset_pos = vec3(i_tex_coords.xy, i_color.a);
    vec3 pos = i_pos + offset_pos;

    float wind_min = radians(5.0);
    float wind_max = radians(30.0);

    // Tried to make it more interactable, did not work
    //vec2 around_pos_min = vec2(pos.x - 0.1f, pos.z - 0.1f);
    //vec2 around_pos_size = vec2(0.2f);
    //vec2 point = vec2(Push.position.x, Push.position.y);
    //if (collides(point, around_pos_min, around_pos_size))
    //{
    //    wind_min -= radians(5.0);
    //}

    pos = vec3((pos.x * offset_increase), 0.0f, (pos.z * offset_increase));

    float angle_noise =
        (sy_value_noise2d(pos.x + (Push.normal[0][0]), pos.z + (Push.normal[0][0]), freq,
                          grain, oct) *
         (wind_max - wind_min)) +
        wind_min;

    angle_noise += i_pos.y * 0.3 * sin(i_tex_index * angle_noise);
    angle_noise *= (i_pos.y + 0.8) * 1.2;

    wind_min = radians(-20.0);
    wind_max = radians(20.0);

    float angle_noise2 =
        (sy_value_noise2d(pos.x + (Push.normal[0][0]), pos.z + (Push.normal[0][0]), freq,
                          grain, oct) *
         (wind_max - wind_min)) +
        wind_min;

    angle_noise2 += i_pos.y * 0.3 * sin(i_tex_index * angle_noise2);
    angle_noise2 *= (i_pos.y + 0.2) * 0.7;

    mat4 m = rotate_x(angle_noise) * rotate_z(angle_noise2) *
             rotate_y(sin(i_tex_index + angle_noise));

    vec4 end_pos = (m * vec4(i_pos, 1.0)) + vec4(offset_pos, 1.0);
    end_pos.w = 1.0;

    //vec3 normal = vec3(inverse(m) * vec4(i_normal, 1.0));
    //vec3 light_pos = vec3(23.0, 25.0, 38.0);
   // vec3 light_dir = normalize(light_pos - vec3(end_pos));
    //float intensity = dot(normal, light_dir);
    vec3 final_color = i_color.rgb;// * intensity;

    gl_Position = VP.proj * VP.view * Push.model * end_pos;
    f_color = vec4(final_color, 1.0);
    f_tex_coord = vec2(0.0);
    f_tex_index = 0;
    f_normal = i_normal;
}

