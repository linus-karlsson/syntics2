#pragma once
#ifndef SY_UNIT_BUILD

#endif
Mutex syntics_platform_mutex_create();
void  syntics_platform_mutex_lock(Mutex* mutex);
void  syntics_platform_mutex_unlock(Mutex* mutex);
void  syntics_platform_mutex_destroy(Mutex* mutex);

Semaphore syntics_platform_semaphore_create(i32 initial_count, i32 max_count);
void      syntics_platform_semaphore_wait_and_decrement(Semaphore* sem);
void      syntics_platform_semaphore_increment(Semaphore* sem);
void     syntics_platform_semaphore_destroy(Semaphore* sem);

Thread_Handle syntics_platform_thread_create(void* data,
                            thread_return_value (*thread_function)(void* data),
                            unsigned long creation_flag,
                            unsigned long* thread_id);
void syntics_platform_thread_join(Thread_Handle handle);
void syntics_platform_thread_destroy(Thread_Handle handle);

u32 syntics_platform_get_core_count();
void platform_title_change(Platform* platform, const char* title, u32 len);
xcb_connection_t* platform_connection_get(Platform* platform);
xcb_window_t syntics_platform_window_get(Platform* platform);
void syntics_platform_init(Region_Alloc* region, const char* title, u16* width,
                   u16* height, b32 full_screen, Platform** platform);
void syntics_platform_set_event_callbacks(
    Platform* platform, void (*on_key_pressed)(u16 key, u16 op),
    void (*on_key_released)(u16 key), void (*on_button_pressed)(u8 key),
    void (*on_button_released)(u8 key),
    void (*on_mouse_move)(i16 pos_x, i16 pos_y),
    void (*on_mouse_wheel)(i16 z_delta), void (*on_window_focused)(b8 focused),
    void (*on_enter_leave)(b8 e_l),
    void (*on_window_resize)(u16 width, u16 height));
void syntics_platform_event_fire(Platform* platform);
void move_main_window(Platform* platform);
void syntics_platform_window_get_size(const Platform* platform, u16* width,
                              u16* height);
void syntics_platform_cursor_hide(const Platform* platform);
void syntics_platform_cursor_show(const Platform* platform);
void syntics_platform_mouse_set_pos(const Platform* platform, i16 pos_x, i16 pos_y);
void syntics_platform_cursor_show_centered(const Platform* platform);
void syntics_platform_mouse_set_last_pos(const Platform* platform);
void syntics_platform_cursor_show_last_pos(const Platform* platform);
void syntics_platform_cursor_change(const Platform* platform, u32 cursor_id);
void syntics_platform_mouse_get_pos(i16* pos_x, i16* pos_y);
u64 syntics_platform_get_time_seed(void);
f64 syntics_platform_get_time(void);
void syntics_platform_sleep(u64 milli);
void syntics_platform_shut_down(Platform* platform);
void syntics_platform_file_read(File_Attrib* file_attrib, Region_Alloc* region,
               const char* file_path);
void syntics_platform_file_write(const char* file_path, const char* mode, const char* content,
                u32 size);
void file_write_append_end(const char* file_path, const char* content);
void syntics_platform_file_write_entire(const char* file_path, const char* content, u32 size);
u32 syntics_platform_get_executable_directory(char* file, u32 size);
void* syntics_platform_virtual_allocation(u64 size);
void syntics_platform_free_allocation(void* mem, u64 capacity);
