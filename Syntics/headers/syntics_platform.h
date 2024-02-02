#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

typedef void (*On_Key_Pressed_Callback)(u16 key);
typedef void (*On_Key_Released_Callback)(u16 key);
typedef void (*On_Button_Pressed_Callback)(u8 key);
typedef void (*On_Button_Released_Callback)(u8 key);
typedef void (*On_Mouse_Moved_Callback)(i16 x, i16 y);
typedef void (*On_Mouse_Wheel_Callback)(i16 z_delta);
typedef void (*On_Window_Focused_Callback)(b8 focused);
typedef void (*On_Window_Resize_Callback)(u16 width, u16 height);
typedef void (*On_Window_Enter_Leave_Callback)(b8 enter_leave);
typedef void (*On_Key_Stroke_Callback)(char key);

void  platform_init(Region_Alloc* region, const char* title, u16* width, u16* height, b32 full_screen, Platform** platform);

Mutex platform_mutex_create(void);
void  platform_mutex_lock(Mutex* mutex);
void  platform_mutex_unlock(Mutex* mutex);
void  platform_mutex_destroy(Mutex* mutex);

Semaphore 
      platform_semaphore_create(i32 initial_count, i32 max_count);
void  platform_semaphore_increment(Semaphore* sem);
void  platform_semaphore_wait_and_decrement(Semaphore* sem);
void  platform_semaphore_destroy(Semaphore* sem);

Thread_Handle 
      platform_thread_create(void* data, thread_return_value (*thread_function)(void* data), unsigned long creation_flag, unsigned long* thread_id);
void  platform_thread_join(Thread_Handle handle);
void  platform_thread_destroy(Thread_Handle handle);

u32   platform_get_core_count(void);
void  platform_error_msg(const char* msg);

#ifdef LINUX
#else
#ifndef SY_UNIT_BUILD
#include "win32/sy_windows.h"
#endif
HWND  platform_window_get(Platform* platform);
b8    platform_window_is_fullscreen(void);
b8    platform_window_is_maximized(void);
void  platform_window_toggle_fullscreen(HWND win);
void  platform_window_toggle_mximized(HWND win);
void  platform_window_move(HWND win, i32 x, i32 y, i32 w, i32 h);
#endif
void  platform_window_get_size(const Platform* platform, u16* width, u16* height);
void  platform_window_get_screen_pos(i32* x, i32* y);

void  platform_event_fire(Platform* platform);
void  platform_event_set_on_key_pressed(Platform* platform, On_Key_Pressed_Callback c);
void  platform_event_set_on_key_released(Platform* platform, On_Key_Released_Callback c);
void  platform_event_set_on_button_pressed(Platform* platform, On_Button_Pressed_Callback c);
void  platform_event_set_on_button_released(Platform* platform, On_Button_Released_Callback c);
void  platform_event_set_on_mouse_move(Platform* platform, On_Mouse_Moved_Callback c);
void  platform_event_set_on_mouse_wheel(Platform* platform, On_Mouse_Wheel_Callback c);
void  platform_event_set_on_window_focused(Platform* platform, On_Window_Focused_Callback c);
void  platform_event_set_on_window_resize(Platform* platform, On_Window_Resize_Callback c);
void  platform_event_set_on_window_enter_leave(Platform* platform, On_Window_Enter_Leave_Callback c);
void  platform_event_set_on_key_stroke(Platform* platform, On_Key_Stroke_Callback c);

void  platform_cursor_set_pos(const Platform* platform, i16 x, i16 y);
void  platform_cursor_hide(const Platform* platform);
void  platform_cursor_show(const Platform* platform);
void  platform_mouse_set_pos(const Platform* platform, i16 pos_x,
                                         i16 pos_y);
void  platform_cursor_show_centered(const Platform* platform);
void  platform_mouse_set_last_pos(const Platform* platform);
void  platform_cursor_show_last_pos(const Platform* platform);
void  platform_cursor_change(const Platform* platform, u32 cursor_id);
void  platform_mouse_get_pos(i16* pos_x, i16* pos_y);
u64   platform_get_time_seed(void);
f64   platform_get_time(void);
void  platform_sleep(u64 milli);
void  platform_shut_down(Platform* platform);

void  platform_file_read(File_Attrib* file_attrib, Region_Alloc* region, const char* file_path);
void  platform_file_write(const char* file_path, const char* content);
void  platform_file_write_entire(const char* file_path, const char* content, u32 size);

u32   platform_get_executable_directory(char* file, u32 size);

void* platform_virtual_allocation(u64 size);
void  platform_free_allocation(void* mem, u64 capacity);

