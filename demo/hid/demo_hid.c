
#include "hid.h"
#include "assert.h"
#include "osi_api.h"

typedef struct
{
    osiTimer_t *timer;
    T_AMOPENAT_HID *h;
} demo_hid_ctx_t;

const uint8_t hid_class_desc[] = 
{
    0x09, 0x21, 0x10, 0x01, 0x00, 0x01, 0x22, 0x36, 0x00,
};

const uint8_t hid_keyboard_report_desc[] =    
{
    0x05, 0x01, 0x09, 0x06, 0xa1, 0x01, 0x05, 0x08,
    0x19, 0x01, 0x29, 0x03, 0x15, 0x00, 0x25, 0x01,
    0x75, 0x01, 0x95, 0x03, 0x91, 0x02, 0x95, 0x05,
    0x91, 0x01, 0x05, 0x07, 0x19, 0xe0, 0x29, 0xe7,
    0x95, 0x08, 0x81, 0x02, 0x75, 0x08, 0x95, 0x01,
    0x81, 0x01, 0x19, 0x00, 0x29, 0x91, 0x26, 0xFF,
    0x00, 0x95, 0x06, 0x81, 0x00, 0xc0
};

int demo_hid_ctrl_req_cb(E_AMOPENAT_HID_CTRL_REQ req, uint16_t w_value, uint8_t *buf, uint16_t w_length, void *ctx)
{
    return 0;
}

int demo_hid_get_descriptor_cb(E_AMOPENAT_HID_DESCRIPTOR type, uint8_t index, uint8_t *buf, uint16_t w_length)
{
    osiTracePrintf(0, "demo hid get descriptor %d", type);
    if (type == OPENAT_HID_DESC_TYPE_HID)
    {
        memcpy(buf, hid_class_desc, sizeof(hid_class_desc));
        return sizeof(hid_class_desc);
    }
    else if (type == OPENAT_HID_DESC_TYPE_REPORT)
    {
        memcpy(buf, hid_keyboard_report_desc, sizeof(hid_keyboard_report_desc));
        return sizeof(hid_keyboard_report_desc);
    }
    return 0;
}

static demo_hid_ctx_t demo_hid_ctx;

static uint8_t demo_hid_report_H[8] = { 0x00, 0x00, HID_KEY_H, 0, 0, 0, 0, 0};
static uint8_t demo_hid_report_E[8] = { 0x00, 0x00, HID_KEY_E, 0, 0, 0, 0, 0};
static uint8_t demo_hid_report_L[8] = { 0x00, 0x00, HID_KEY_L, 0, 0, 0, 0, 0};
static uint8_t demo_hid_report_O[8] = { 0x00, 0x00, HID_KEY_O, 0, 0, 0, 0, 0};
static uint8_t demo_hid_report_0[8] = { 0 };

uint8_t *reports[5];

static void _demo_hid_generate_report(demo_hid_ctx_t *h)
{
	static uint8_t i = 0;
    T_AMOPENAT_HID_REPORT *r = OPENAT_hid_tx_req_alloc(h->h);
    if (!r)
		return;
    memcpy(r->payload, reports[i%5], 8);
    r->payload_size = 8;
    if (!OPENAT_hid_tx_req_submit(h->h, r))
        OPENAT_hid_tx_req_free(h->h, r);
	i++;
	r = OPENAT_hid_tx_req_alloc(h->h);
	if (!r)
		return;
	uint8_t zero[8] = { 0 };
	memset(r->payload, zero, sizeof(zero));
	r->payload_size = sizeof(zero);
	if (!OPENAT_hid_tx_req_submit(h->h, r))
		OPENAT_hid_tx_req_free(h->h, r);
}

static void _demo_hid_task(demo_hid_ctx_t *h)
{
    osiDelayUS(1000 * 2000);
    osiTimerStartPeriodic(h->timer, 2000);
    osiEvent_t e;
    for (;;)
    {
        osiEventWait(osiThreadCurrent(), &e);
        if (e.id == OSI_EVENT_ID_QUIT)
            break;
    }
    osiThreadExit();
}

void demo_hid_enable()
{
    T_AMOPENAT_HID_CONFIG cnf;
    memset(&cnf, 0, sizeof(T_AMOPENAT_HID_CONFIG));
    cnf.subclass = 1;
    cnf.protocol = 1;
    cnf.ops.ctrl_req_cb = demo_hid_ctrl_req_cb;
    cnf.ops.get_desc_cb = demo_hid_get_descriptor_cb;
    cnf.ops.cb_ctx = &demo_hid_ctx;

    T_AMOPENAT_HID *hid = OPENAT_hid_func_create(cnf);
    // assert(hid != NULL);
    demo_hid_ctx.h = hid;
	OPENAT_hid_only(hid, true);
    bool suc = OPENAT_hid_func_enable(hid);
    // assert(suc);

    osiThread_t *t = osiThreadCreate("hid", _demo_hid_task, &demo_hid_ctx, OSI_PRIORITY_NORMAL, 4096, 1024);
    demo_hid_ctx.timer = osiTimerCreate(t, _demo_hid_generate_report, &demo_hid_ctx);
}

int appimg_enter(void *param)
{
	reports[0] = demo_hid_report_H;
	reports[1] = demo_hid_report_E;
	reports[2] = demo_hid_report_L;
	reports[3] = demo_hid_report_L;
	reports[4] = demo_hid_report_O;
    demo_hid_enable();
    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[os] appimg_exit");
}
