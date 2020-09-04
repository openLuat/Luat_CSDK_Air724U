

#include "iot_debug.h"
#include "am_openat_tts.h"
#include "iot_tts.h"
static bool tts_play_init = FALSE;

static bool tts_play_status = FALSE;

static void TTS_test_cb(OPENAT_TTS_CB_MSG msg_id, u8 event)
{
    if (msg_id == OPENAT_TTS_CB_MSG_STATUS)
    {
        tts_play_status = FALSE;
        iot_tts_stop();
        iot_debug_print("[coreTest-tts] TTS_test_cb OPENAT_TTS_CB_MSG_STATUS OK:%d", event);
    }
    else if (msg_id == OPENAT_TTS_CB_MSG_ERROR)
    {
        iot_debug_print("[coreTest-False-tts] TTS_test_cb OPENAT_TTS_CB_MSG_ERROR err:%d", event);
    }
}

bool ttsTest(char *text, u32 len)
{
    if (tts_play_status == TRUE)
    {
        iot_debug_print("[coreTest-False-tts] ttsTest be busy");
        return FALSE;
    }
    if (tts_play_init == FALSE)
    {
        iot_tts_init(TTS_test_cb);
        tts_play_init = TRUE;
    }
    iot_tts_play(text, len);
    tts_play_status = TRUE;
    return TRUE;
}