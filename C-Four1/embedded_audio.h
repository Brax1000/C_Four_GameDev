#ifndef EMBEDDED_AUDIO_H
#define EMBEDDED_AUDIO_H

#include <stddef.h>

typedef struct {
    const unsigned char *data;
    size_t size;
} EmbeddedAudioBuffer;

/* Embedded Background Music */
static const unsigned char EMBED_BGM_DATA[] = {
#embed "assets/background.mp3"
};

/* Embedded SFX */
static const unsigned char EMBED_SFX_BOOT_DATA[] = {
#embed "assets/sfx_boot.wav"
};
static const unsigned char EMBED_SFX_KEYSTROKE_DATA[] = {
#embed "assets/sfx_keystroke.wav"
};
static const unsigned char EMBED_SFX_NAV_DATA[] = {
#embed "assets/sfx_nav.wav"
};
static const unsigned char EMBED_SFX_SELECT_DATA[] = {
#embed "assets/sfx_select.wav"
};
static const unsigned char EMBED_SFX_BACK_DATA[] = {
#embed "assets/sfx_back.wav"
};
static const unsigned char EMBED_SFX_KEYPAD_0_DATA[] = {
#embed "assets/sfx_keypad_0.wav"
};
static const unsigned char EMBED_SFX_KEYPAD_1_DATA[] = {
#embed "assets/sfx_keypad_1.wav"
};
static const unsigned char EMBED_SFX_KEYPAD_2_DATA[] = {
#embed "assets/sfx_keypad_2.wav"
};
static const unsigned char EMBED_SFX_KEYPAD_3_DATA[] = {
#embed "assets/sfx_keypad_3.wav"
};
static const unsigned char EMBED_SFX_KEYPAD_4_DATA[] = {
#embed "assets/sfx_keypad_4.wav"
};
static const unsigned char EMBED_SFX_KEYPAD_5_DATA[] = {
#embed "assets/sfx_keypad_5.wav"
};
static const unsigned char EMBED_SFX_KEYPAD_6_DATA[] = {
#embed "assets/sfx_keypad_6.wav"
};
static const unsigned char EMBED_SFX_KEYPAD_7_DATA[] = {
#embed "assets/sfx_keypad_7.wav"
};
static const unsigned char EMBED_SFX_KEYPAD_8_DATA[] = {
#embed "assets/sfx_keypad_8.wav"
};
static const unsigned char EMBED_SFX_KEYPAD_9_DATA[] = {
#embed "assets/sfx_keypad_9.wav"
};
static const unsigned char EMBED_SFX_DELETE_DATA[] = {
#embed "assets/sfx_delete.wav"
};
static const unsigned char EMBED_SFX_CLEAR_DATA[] = {
#embed "assets/sfx_clear.wav"
};
static const unsigned char EMBED_SFX_ERROR_DATA[] = {
#embed "assets/sfx_error.wav"
};
static const unsigned char EMBED_SFX_HIGHER_DATA[] = {
#embed "assets/sfx_higher.wav"
};
static const unsigned char EMBED_SFX_LOWER_DATA[] = {
#embed "assets/sfx_lower.wav"
};
static const unsigned char EMBED_SFX_TICK_DATA[] = {
#embed "assets/sfx_tick.wav"
};
static const unsigned char EMBED_SFX_ALARM_DATA[] = {
#embed "assets/sfx_alarm.wav"
};
static const unsigned char EMBED_SFX_THEME_DATA[] = {
#embed "assets/sfx_theme.wav"
};
static const unsigned char EMBED_SFX_PAUSE_DATA[] = {
#embed "assets/sfx_pause.wav"
};
static const unsigned char EMBED_SFX_RESUME_DATA[] = {
#embed "assets/sfx_resume.wav"
};
static const unsigned char EMBED_SFX_EXPLOSION_DATA[] = {
#embed "assets/sfx_explosion.wav"
};
static const unsigned char EMBED_SFX_WIN_DATA[] = {
#embed "assets/sfx_win.wav"
};
static const unsigned char EMBED_SFX_STAR_DATA[] = {
#embed "assets/sfx_star.wav"
};

static const EmbeddedAudioBuffer EMBEDDED_SFX_BUFFERS[] = {
    { EMBED_SFX_BOOT_DATA, sizeof(EMBED_SFX_BOOT_DATA) },
    { EMBED_SFX_KEYSTROKE_DATA, sizeof(EMBED_SFX_KEYSTROKE_DATA) },
    { EMBED_SFX_NAV_DATA, sizeof(EMBED_SFX_NAV_DATA) },
    { EMBED_SFX_SELECT_DATA, sizeof(EMBED_SFX_SELECT_DATA) },
    { EMBED_SFX_BACK_DATA, sizeof(EMBED_SFX_BACK_DATA) },
    { EMBED_SFX_KEYPAD_0_DATA, sizeof(EMBED_SFX_KEYPAD_0_DATA) },
    { EMBED_SFX_KEYPAD_1_DATA, sizeof(EMBED_SFX_KEYPAD_1_DATA) },
    { EMBED_SFX_KEYPAD_2_DATA, sizeof(EMBED_SFX_KEYPAD_2_DATA) },
    { EMBED_SFX_KEYPAD_3_DATA, sizeof(EMBED_SFX_KEYPAD_3_DATA) },
    { EMBED_SFX_KEYPAD_4_DATA, sizeof(EMBED_SFX_KEYPAD_4_DATA) },
    { EMBED_SFX_KEYPAD_5_DATA, sizeof(EMBED_SFX_KEYPAD_5_DATA) },
    { EMBED_SFX_KEYPAD_6_DATA, sizeof(EMBED_SFX_KEYPAD_6_DATA) },
    { EMBED_SFX_KEYPAD_7_DATA, sizeof(EMBED_SFX_KEYPAD_7_DATA) },
    { EMBED_SFX_KEYPAD_8_DATA, sizeof(EMBED_SFX_KEYPAD_8_DATA) },
    { EMBED_SFX_KEYPAD_9_DATA, sizeof(EMBED_SFX_KEYPAD_9_DATA) },
    { EMBED_SFX_DELETE_DATA, sizeof(EMBED_SFX_DELETE_DATA) },
    { EMBED_SFX_CLEAR_DATA, sizeof(EMBED_SFX_CLEAR_DATA) },
    { EMBED_SFX_ERROR_DATA, sizeof(EMBED_SFX_ERROR_DATA) },
    { EMBED_SFX_HIGHER_DATA, sizeof(EMBED_SFX_HIGHER_DATA) },
    { EMBED_SFX_LOWER_DATA, sizeof(EMBED_SFX_LOWER_DATA) },
    { EMBED_SFX_TICK_DATA, sizeof(EMBED_SFX_TICK_DATA) },
    { EMBED_SFX_ALARM_DATA, sizeof(EMBED_SFX_ALARM_DATA) },
    { EMBED_SFX_THEME_DATA, sizeof(EMBED_SFX_THEME_DATA) },
    { EMBED_SFX_PAUSE_DATA, sizeof(EMBED_SFX_PAUSE_DATA) },
    { EMBED_SFX_RESUME_DATA, sizeof(EMBED_SFX_RESUME_DATA) },
    { EMBED_SFX_EXPLOSION_DATA, sizeof(EMBED_SFX_EXPLOSION_DATA) },
    { EMBED_SFX_WIN_DATA, sizeof(EMBED_SFX_WIN_DATA) },
    { EMBED_SFX_STAR_DATA, sizeof(EMBED_SFX_STAR_DATA) }
};

#endif
