#pragma once

#include "defines.h"

namespace synt {

#define GUI_DEACTIVATED 12345
#define NUM_GUI_WINDOWS 20

typedef struct Gui_Window Gui_Window;
struct Gui_Window
{
    Gui_Window();
    Gui_Window(const char* title, uint32 width, uint32 height, uint32 pos_x,
               uint32 pos_y);
    ~Gui_Window();

    void create_window(const char* title, uint32 width, uint32 height,
                       uint32 pos_x, uint32 pos_y);

    void destroy_window();

    const uint32 id() const { return m_id; }

private:
    uint32 m_id;
};

} // namespace synt
