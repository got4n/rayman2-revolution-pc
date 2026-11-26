#ifndef PC_PC_H
#define PC_PC_H

#include <stdint.h>

#define PC_API

typedef uint32_t PcAddress;

enum {
    PC_PAD_SELECT = 0x0001,
    PC_PAD_L3 = 0x0002,
    PC_PAD_R3 = 0x0004,
    PC_PAD_START = 0x0008,
    PC_PAD_UP = 0x0010,
    PC_PAD_RIGHT = 0x0020,
    PC_PAD_DOWN = 0x0040,
    PC_PAD_LEFT = 0x0080,
    PC_PAD_L2 = 0x0100,
    PC_PAD_R2 = 0x0200,
    PC_PAD_L1 = 0x0400,
    PC_PAD_R1 = 0x0800,
    PC_PAD_TRIANGLE = 0x1000,
    PC_PAD_CIRCLE = 0x2000,
    PC_PAD_CROSS = 0x4000,
    PC_PAD_SQUARE = 0x8000,
};

#define PC_PAD_AXIS_CENTER 128

typedef struct PcPad {
    uint16_t buttons;
    uint8_t leftX;
    uint8_t leftY;
    uint8_t rightX;
    uint8_t rightY;
} PcPad;

PC_API int32_t pc_pad_read(int32_t port, PcPad *pad);

typedef struct PcFile {
    uint32_t sector;
    uint32_t size;
} PcFile;

#define PC_FILE_SECTOR_SIZE 2048

PC_API int32_t pc_file_find(const char *path, PcFile *file);

PC_API int32_t pc_file_read(const PcFile *file, uint32_t offset, void *buffer, uint32_t size);

typedef struct PcDate {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} PcDate;

PC_API int32_t pc_clock_read(PcDate *date);

typedef struct PcSaveEntry {
    char name[32];
    uint32_t size;
    uint8_t isDirectory;
    PcDate created;
    PcDate modified;
} PcSaveEntry;

PC_API int32_t pc_save_list(const char *path, PcSaveEntry *entries, int32_t maxEntries);

PC_API int32_t pc_save_make_directory(const char *path);
PC_API int32_t pc_save_delete(const char *path);
PC_API int32_t pc_save_read(const char *path, void *buffer, uint32_t size);
PC_API int32_t pc_save_write(const char *path, const void *buffer, uint32_t size);

PC_API int32_t pc_frame_vblanks(void);

PC_API int32_t pc_frame_units_per_tick(void);

PC_API int32_t pc_frame_tick_parts(void);

PC_API void pc_frame_wait(int32_t vblanks);

PC_API void pc_frame_start(void);

PC_API void pc_frame_wait_vblank(void);

PC_API void pc_frame_vblank_signal(int32_t sema);

PC_API void pc_frame_wait_half(void);

PC_API int32_t pc_inbetween_wanted(void);

PC_API void pc_inbetween_keep_begin(void);

PC_API void pc_inbetween_keep(PcAddress superObject, int32_t fresh);

PC_API int32_t pc_inbetween_move(void);

PC_API void pc_inbetween_restore(void);

PC_API int32_t pc_hud_narrow(void);


/* The game's text languages, as the PS2's system language (sceScfGetLanguage) gives them. */
enum {
    PC_LANGUAGE_ENGLISH = 1,
    PC_LANGUAGE_FRENCH = 2,
    PC_LANGUAGE_SPANISH = 3,
    PC_LANGUAGE_GERMAN = 4,
    PC_LANGUAGE_ITALIAN = 5,
};

PC_API int32_t pc_language(void);

PC_API int32_t pc_draw_distance(void);

PC_API uint64_t pc_time_ticks(uint32_t frequency);

PC_API int32_t pc_thread_create(PcAddress entry, PcAddress stack, uint32_t stackSize, int32_t priority);

PC_API int32_t pc_thread_start(int32_t thread, uint32_t argument);

PC_API int32_t pc_thread_stop(int32_t thread);

PC_API int32_t pc_thread_delete(int32_t thread);

PC_API int32_t pc_thread_self(void);

PC_API int32_t pc_thread_priority(int32_t thread);

PC_API void pc_thread_set_priority(int32_t thread, int32_t priority);

PC_API void pc_thread_sleep(void);

PC_API void pc_thread_wake(int32_t thread);

PC_API int32_t pc_sema_create(int32_t count, int32_t max);

PC_API void pc_sema_delete(int32_t sema);

PC_API void pc_sema_wait(int32_t sema);

PC_API void pc_sema_signal(int32_t sema);

PC_API int32_t pc_sound_start(void);

PC_API void pc_sound_command(uint32_t function, PcAddress request, uint32_t requestSize, PcAddress reply,
                             uint32_t replySize);

PC_API int32_t pc_sound_receive(uint32_t *function, void *buffer, uint32_t size, uint32_t *messageSize);

PC_API void pc_sound_wait(void);

PC_API int32_t pc_movie_open(const void *stream, uint32_t size);

PC_API int32_t pc_movie_picture(void *pixels, uint32_t width, uint32_t height);

PC_API int32_t pc_movie_ended(void);

PC_API int32_t pc_movie_size(uint32_t *width, uint32_t *height);

PC_API void pc_movie_close(void);

extern unsigned char pc_scratchpad[0x4000];
#define PC_SCRATCH(a) ((void *)(pc_scratchpad + ((unsigned)(a) & 0x3FFFu)))

typedef struct PcVu0 {
    float vf[32][4];
    float acc[4];
    float q;
    uint32_t vi[16];
} PcVu0;

extern PcVu0 pc_vu0;

PC_API void pc_vu0_load(PcAddress program);

PC_API void pc_vu0_call(uint32_t address, PcVu0 *registers);

PC_API int32_t pc_mod_skip_wait(int32_t ticks);

PC_API int32_t pc_mod_fade_frames(int32_t frames);

PC_API int32_t pc_mod_skip_vblank_waits(void);

PC_API int32_t pc_mod_fade_in_step(int32_t step);

enum {
    PC_LOAD_LEAVE = 1,
    PC_LOAD_FADED,
    PC_LOAD_LEFT,
    PC_LOAD_BEGIN,
    PC_LOAD_MOVIE_ON,
    PC_LOAD_FILES,
    PC_LOAD_OBJECTS,
    PC_LOAD_RW,
    PC_LOAD_MOVIE_OFF,
    PC_LOAD_MOVIE_DONE,
    PC_LOAD_SKY_CACHE,
    PC_LOAD_END,
    PC_LOAD_READY,
};
PC_API void pc_mod_load_mark(int32_t step);
PC_API void pc_mod_load_frame(int32_t frame);

PC_API int32_t pc_mod_any_button_now(void);

PC_API int32_t pc_mod_entry_now(PcAddress entry);
PC_API int32_t pc_mod_button_condition(int32_t kind, PcAddress entry, int32_t value);


PC_API void pc_mod_text(int32_t id, PcAddress text);

PC_API int32_t pc_mod_autosave_prompt(PcAddress condition);
PC_API PcAddress pc_mod_autosave_clock(PcAddress condition);
PC_API void pc_mod_autosave_done(int32_t saved);

PC_API void pc_mod_camera_updated(PcAddress update);

/* The Extras tab's cheats (host/mods/mod_cheats.c). */
PC_API int32_t pc_mod_mechanics_percent(PcAddress superObject);
PC_API int32_t pc_mod_no_damage(PcAddress engineObject);
PC_API int32_t pc_mod_noclip(PcAddress dynamics, PcAddress superObject);
PC_API int32_t pc_mod_noclip_camera(PcAddress position, PcAddress target);
PC_API int32_t pc_mod_noclip_active(PcAddress superObject);
PC_API int32_t pc_mod_camera_percent(void);


#endif
