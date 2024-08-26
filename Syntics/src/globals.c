#include "globals.h"

global V4 g_clear_color = {
    .r = 0.1f,
    .g = 0.1f,
    .b = 0.1f,
    .a = 1.0f,
};

global V4 g_high_light_color = {
    .r = 0.2f,
    .g = 0.2f,
    .b = 0.2f,
    .a = 1.0f,
};

global V4 g_border_color = {
    .r = 0.35f,
    .g = 0.35f,
    .b = 0.35f,
    .a = 1.0f,
};

global V4 g_lighter_color = {
    .r = 0.55f,
    .g = 0.55f,
    .b = 0.55f,
    .a = 1.0f,
};
global V4 g_bright_color = {
    .r = 0.7f,
    .g = 0.7f,
    .b = 0.7f,
    .a = 1.0f,
};
global V4 g_secondary_color = {
    .r = 0.0f,
    .g = 0.58f,
    .b = 1.0f,
    .a = 1.0f,
};

global V4 g_text_color = {
    .r = 1.0f,
    .g = 1.0f,
    .b = 1.0f,
    .a = 1.0f,
};

global V4 g_tab_color = {
    .r = 0.25f,
    .g = 0.25f,
    .b = 0.25f,
    .a = 1.0f,
};

global V4 g_bar_top_color = {
    .r = 0.25f,
    .g = 0.25f,
    .b = 0.25f,
    .a = 1.0f,
};

global V4 g_bar_bottom_color = {
    .r = 0.16f,
    .g = 0.16f,
    .b = 0.16f,
    .a = 1.0f,
};

global V4 g_scroll_bar_color = {
    .r = 0.55f,
    .g = 0.55f,
    .b = 0.55f,
    .a = 1.0f,
};

global f32 g_border_width = 1.0f;

V4 global_get_clear_color(void)
{
    return g_clear_color;
}

V4 global_get_highlight_color(void)
{
    return g_high_light_color;
}

V4 global_get_border_color(void)
{
    return g_border_color;
}

V4 global_get_lighter_color(void)
{
    return g_lighter_color;
}

V4 global_get_bright_color(void)
{
    return g_bright_color;
}

V4 global_get_secondary_color(void)
{
    return g_secondary_color;
}

V4 global_get_text_color(void)
{
    return g_text_color;
}

V4 global_get_tab_color(void)
{
    return g_tab_color;
}

V4 global_get_bar_top_color(void)
{
    return g_bar_top_color;
}

V4 global_get_bar_bottom_color(void)
{
    return g_bar_bottom_color;
}

V4 global_get_scroll_bar_color(void)
{
    return g_scroll_bar_color;
}

f32 global_get_border_width(void)
{
    return g_border_width;
}

void global_set_clear_color(V4 v)
{
    g_clear_color = v;
}

void global_set_highlight_color(V4 v)
{
    g_high_light_color = v;
}

void global_set_border_color(V4 v)
{
    g_border_color = v;
}

void global_set_lighter_color(V4 v)
{
    g_lighter_color = v;
}

void global_set_bright_color(V4 v)
{
    g_bright_color = v;
}

void global_set_secondary_color(V4 v)
{
    g_secondary_color = v;
}

void global_set_text_color(V4 v)
{
    g_text_color = v;
}

void global_set_tab_color(V4 v)
{
    g_tab_color = v;
}

void global_set_bar_top_color(V4 v)
{
    g_bar_top_color = v;
}

void global_set_bar_bottom_color(V4 v)
{
    g_bar_bottom_color = v;
}

void global_set_scroll_bar_color(V4 v)
{
    g_scroll_bar_color = v;
}

void global_set_border_width(f32 v)
{
    g_border_width = v;
}
