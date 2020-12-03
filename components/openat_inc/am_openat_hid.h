#ifndef AM_OPENAT_HID_H
#define AM_OPENAT_HID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum E_AMOPENAT_HID_SUBCLASS_TAG
{
    OPENAT_HID_SUBCLASS_NONE = 0,
    OPENAT_HID_SUBCLASS_BOOT = 1,
} E_AMOPENAT_HID_SUBCLASS;

typedef enum E_AMOPENAT_HID_PROTOCOL_TAG
{
    OPENAT_HID_PROTOCOL_NONE     = 0,
    OPENAT_HID_PROTOCOL_KEYBOARD = 1,
    OPENAT_HID_PROTOCOL_MOUSE    = 2,
} E_AMOPENAT_HID_PROTOCOL;

typedef enum E_AMOPENAT_HID_DESCRIPTOR_TYPE_TAG
{
    OPENAT_HID_DESC_TYPE_HID      = 0x21, ///< HID Descriptor
    OPENAT_HID_DESC_TYPE_REPORT   = 0x22, ///< Report Descriptor
    OPENAT_HID_DESC_TYPE_PHYSICAL = 0x23  ///< Physical Descriptor
} E_AMOPENAT_HID_DESCRIPTOR;

typedef enum E_AM_OPENAT_HID_REPORT_TYPE_TAG
{
    OPENAT_HID_REPORT_TYPE_INVALID = 0,
    OPENAT_HID_REPORT_TYPE_INPUT,      ///< Input
    OPENAT_HID_REPORT_TYPE_OUTPUT,     ///< Output
    OPENAT_HID_REPORT_TYPE_FEATURE     ///< Feature
} E_AMOPENAT_HID_REPORT_TYPE;

typedef enum E_AMOPENAT_HID_CTRL_REQ_TAG
{
    OPENAT_HID_REQ_CONTROL_GET_REPORT   = 0x01, ///< Get Report
    OPENAT_HID_REQ_CONTROL_GET_IDLE     = 0x02, ///< Get Idle
    OPENAT_HID_REQ_CONTROL_GET_PROTOCOL = 0x03, ///< Get Protocol
    OPENAT_HID_REQ_CONTROL_SET_REPORT   = 0x09, ///< Set Report
    OPENAT_HID_REQ_CONTROL_SET_IDLE     = 0x0a, ///< Set Idle
    OPENAT_HID_REQ_CONTROL_SET_PROTOCOL = 0x0b  ///< Set Protocol
} E_AMOPENAT_HID_CTRL_REQ;

typedef struct T_AMOPENAT_HID_TAG T_AMOPENAT_HID;

typedef struct T_AMOPENAT_HID_REPORT_TAG
{
    uint8_t *payload;
    uint32_t payload_size;
} T_AMOPENAT_HID_REPORT;

typedef int (*ctrl_req_cb_t)(E_AMOPENAT_HID_CTRL_REQ req, uint16_t w_value, uint8_t *buf, uint16_t w_length, void *ctx);

typedef void (*outep_data_cb_t)(/*TODO: complete func property*/);

typedef int (*get_descriptor_cb_t)(E_AMOPENAT_HID_DESCRIPTOR type, uint8_t index, uint8_t *buf, uint16_t w_length, void *ctx);

typedef struct T_AMOPENAT_HID_CONFIG_TAG
{
    E_AMOPENAT_HID_SUBCLASS subclass;
    E_AMOPENAT_HID_PROTOCOL protocol;
    struct
    {
        ctrl_req_cb_t ctrl_req_cb;
        outep_data_cb_t uldata_cb;
        get_descriptor_cb_t get_desc_cb;
        void *cb_ctx;
    } ops;
} T_AMOPENAT_HID_CONFIG;

static inline void OPENAT_set_uldata_cb(T_AMOPENAT_HID_CONFIG *hid, outep_data_cb_t cb)
{
    hid->ops.uldata_cb = cb;
}

//创建HID设备
T_AMOPENAT_HID *OPENAT_hid_func_create(T_AMOPENAT_HID_CONFIG cnf);

//销毁HID设备
void OPENAT_hid_func_destroy(T_AMOPENAT_HID *hid);

//使能HID设备
bool OPENAT_hid_func_enable(T_AMOPENAT_HID *hid);

//失能HID设备
void OPENAT_hid_func_disable(T_AMOPENAT_HID *hid);

//分配一个提交缓存区
T_AMOPENAT_HID_REPORT *OPENAT_hid_tx_req_alloc(T_AMOPENAT_HID *hid);

//释放一个提交缓存区
void OPENAT_hid_tx_req_free(T_AMOPENAT_HID *hid, T_AMOPENAT_HID_REPORT *req);

//提交信息
bool OPENAT_hid_tx_req_submit(T_AMOPENAT_HID *hid, T_AMOPENAT_HID_REPORT *req);

//使能仅HID模式
void OPENAT_hid_only(T_AMOPENAT_HID *h, bool enable);

#ifdef __cplusplus
}
#endif

#endif