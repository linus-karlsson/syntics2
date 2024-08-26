#pragma once
#ifndef SY_UNIT_BUILD
#include "math/syntics_math.h"
#endif

V4 global_get_clear_color(void);
V4 global_get_highlight_color(void);
V4 global_get_border_color(void);
V4 global_get_lighter_color(void);
V4 global_get_bright_color(void);
V4 global_get_secondary_color(void);
V4 global_get_text_color(void);
V4 global_get_tab_color(void);
V4 global_get_bar_top_color(void);
V4 global_get_bar_bottom_color(void);
V4 global_get_scroll_bar_color(void);
f32 global_get_border_width(void);

void global_set_clear_color(V4 v);
void global_set_highlight_color(V4 v);
void global_set_border_color(V4 v);
void global_set_lighter_color(V4 v);
void global_set_bright_color(V4 v);
void global_set_secondary_color(V4 v);
void global_set_text_color(V4 v);
void global_set_tab_color(V4 v);
void global_set_bar_top_color(V4 v);
void global_set_bar_bottom_color(V4 v);
void global_set_scroll_bar_color(V4 v);
void global_set_border_width(f32 v);
