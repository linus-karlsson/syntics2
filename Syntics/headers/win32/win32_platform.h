#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#include "win32/sy_windows.h"
#endif

Mutex mutex_create();
void mutex_lock(Mutex* mutex);
void mutex_unlock(Mutex* mutex);
void mutex_destroy(Mutex* mutex);
Semaphore semaphore_create(i32 initial_count, i32 max_count);
void semaphore_wait_and_decrement(Semaphore* sem);
void semaphore_increment(Semaphore* sem);
void semaphore_destroy(Semaphore* sem);
Thread_Handle thread_create(void* data,
                            thread_return_value (*thread_function)(void* data),
                            unsigned long creation_flag,
                            unsigned long* thread_id);
void thread_join(Thread_Handle handle);
void thread_destroy(Thread_Handle handle);
u32 platform_core_count();
void error_msg(const char* msg);
HWND platform_window_get(Platform* platform);
LRESULT msg_handler(HWND win, UINT msg, WPARAM w_param, LPARAM l_param);
void platform_init(Region_Alloc* region, const char* title, u16* width,
                   u16* height, b32 full_screen, Platform** platform);
void platform_event_set_callbacks(
    Platform* platform, void (*on_key_pressed)(u16 key),
    void (*on_key_released)(u16 key), void (*on_button_pressed)(u8 key),
    void (*on_button_released)(u8 key),
    void (*on_mouse_move)(i16 pos_x, i16 pos_y),
    void (*on_mouse_wheel)(i16 z_delta), void (*on_window_focused)(b8 focused),
    void (*on_enter_leave)(b8 e_l),
    void (*on_window_resize)(u16 width, u16 height), 
    void (*on_key_stroke)(char key));
b8 is_fullscreen(void);
b8 is_maximized(void);
void sy_toggle_fullscreen(HWND win);
void sy_toggle_maximize(HWND win);
void window_move(HWND win, i32 x, i32 y, i32 w, i32 h);
void event_fire(Platform* platform);
void platform_window_get_size(const Platform* platform, u16* width,
                              u16* height);
void screen_get_pos(i32* x, i32* y);
void platform_cursor_set_pos(const Platform* platform, i16 x, i16 y);
void platform_cursor_hide(const Platform* platform);
void platform_cursor_show(const Platform* platform);
void platform_mouse_set_pos(const Platform* platform, i16 pos_x, i16 pos_y);
void platform_cursor_show_centered(const Platform* platform);
void platform_mouse_set_last_pos(const Platform* platform);
void platform_cursor_show_last_pos(const Platform* platform);
void platform_cursor_change(const Platform* platform, u32 cursor_id);
void platform_mouse_get_pos(i16* pos_x, i16* pos_y);
u64 platform_get_time_nano();
f64 platform_get_time(void);
void platform_sleep(u64 milli);
void platform_shut_down(Platform* platform);
HANDLE file_get_handle(LPCSTR file_path, DWORD operation, DWORD share_mode,
                       DWORD creation);
u32 file_get_size(HANDLE file);
void file_read_bytes(File_Attrib* file_attrib, HANDLE file);
void file_read(File_Attrib* file_attrib, Region_Alloc* region,
               const char* file_path);
void file_write(const char* file_path, const char* content);
void file_write_entire(const char* file_path, const char* content, u32 size);
u32 executable_directory(char* file, u32 size);
void* virtual_allocation(u64 size);
void free_allocation(void* mem, u64 capacity);
