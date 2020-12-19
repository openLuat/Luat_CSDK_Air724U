/***************
	demo_hello
****************/
#include <string.h>
#include "iot_debug.h"
#include "iot_os.h"

#include "am_openat_drv.h"

#include "iot_vat.h"

#include "iot_keypad.h"

VOID keypad_cb(T_AMOPENAT_KEYPAD_MESSAGE *pKeypadMessage)
{
    iot_debug_print("[poc-key] pKeypadMessage->bPressed :%d", pKeypadMessage->bPressed);
    iot_debug_print("[poc-key] pKeypadMessage->data.matrix.r :%d", pKeypadMessage->data.matrix.r);
    iot_debug_print("[poc-key] pKeypadMessage->data.matrix.c :%d", pKeypadMessage->data.matrix.c);
}

int appimg_enter(void *param)
{
    iot_debug_set_fault_mode(OPENAT_FAULT_HANG);
    //打开调试信息，默认关闭   
    iot_vat_send_cmd((UINT8 *)"AT^TRACECTRL=0,1,1\r\n", sizeof("AT^TRACECTRL=0,1,1\r\n"));

    T_AMOPENAT_KEYPAD_CONFIG keypadConfig = {0};
    //pConfig.config=
    keypadConfig.type = OPENAT_KEYPAD_TYPE_MATRIX;
    keypadConfig.pKeypadMessageCallback = keypad_cb;
    keypadConfig.config.matrix.keyInMask = 0x0f;
    keypadConfig.config.matrix.keyOutMask = 0x0f;
    BOOL err = iot_keypad_init(&keypadConfig);
    if (!err)
        iot_debug_print("[poc-key] iot_keypad_init false!");

    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[poc-key] appimg_exit");
}
