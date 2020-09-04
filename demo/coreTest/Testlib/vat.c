#include "string.h"
#include "iot_debug.h"
#include "iot_vat.h"

static AtCmdRsp AtCmdCb_wimei(char *pRspStr)
{
    iot_debug_print("[coreTest-vat]:AtCmdCb_wimei");
    AtCmdRsp rspValue = AT_RSP_WAIT;
    char *rspStrTable[] = {"+CME ERROR", "+WIMEI:", "OK"};
    s16 rspType = -1;
    char imei[16] = {0};
    u8 i = 0;
    char *p = pRspStr + 2;
    for (i = 0; i < sizeof(rspStrTable) / sizeof(rspStrTable[0]); i++)
    {
        if (!strncmp(rspStrTable[i], p, strlen(rspStrTable[i])))
        {
            rspType = i;
            if (rspType == 1)
            {
                strncpy(imei, p + strlen(rspStrTable[i]), 15);
                iot_debug_print("[coreTest-vat]:imei: %s", imei);
            }
            break;
        }
    }
    switch (rspType)
    {
    case 0: /* ERROR */
        rspValue = AT_RSP_ERROR;
        break;

    case 1: /* +wimei */
        rspValue = AT_RSP_WAIT;
        break;

    case 2: /* OK */
        rspValue = AT_RSP_CONTINUE;
        break;
    default:
        break;
    }
    return rspValue;
}

static AtCmdRsp AtCmdCb_iccid(char *pRspStr)
{
    iot_debug_print("[coreTest-vat]:AtCmdCb_iccid");
    AtCmdRsp rspValue = AT_RSP_WAIT;
    char *rspStrTable[] = {"ERROR", "+ICCID:", "OK"};
    s16 rspType = -1;
    char iccid[20] = {0};
    u8 i = 0;
    char *p = pRspStr + 2;
    for (i = 0; i < sizeof(rspStrTable) / sizeof(rspStrTable[0]); i++)
    {
        if (!strncmp(rspStrTable[i], p, strlen(rspStrTable[i])))
        {
            rspType = i;
            if (rspType == 1)
            {
                strncpy(iccid, p + strlen(rspStrTable[i]), 20);
            }
            break;
        }
    }
    switch (rspType)
    {
    case 0: /* ERROR */
        rspValue = AT_RSP_ERROR;
        break;

    case 1: /* iccid */
        rspValue = AT_RSP_WAIT;
        break;

    case 2: /* OK */
        rspValue = AT_RSP_CONTINUE;
        break;

    default:
        break;
    }
    return rspValue;
}

static AtCmdRsp AtCmdCb_cimi(char *pRspStr)
{
    iot_debug_print("[coreTest-vat]:AtCmdCb_cimi");
    AtCmdRsp rspValue = AT_RSP_WAIT;
    char *rspStrTable[] = {"+CME ERROR", "460", "OK"};
    s16 rspType = -1;
    char imsi[20] = {0};
    u8 i = 0;
    char *p = pRspStr + 2;
    for (i = 0; i < sizeof(rspStrTable) / sizeof(rspStrTable[0]); i++)
    {
        if (!strncmp(rspStrTable[i], p, strlen(rspStrTable[i])))
        {
            rspType = i;
            if (rspType == 1)
            {
                strncpy(imsi, p, 15);
                iot_debug_print("[coreTest-vat]:imsi: %s", imsi);
            }
            break;
        }
    }

    switch (rspType)
    {
    case 0: /* ERROR */
        rspValue = AT_RSP_ERROR;
        break;

    case 1: /* imsi */
        rspValue = AT_RSP_WAIT;
        break;

    case 2: /* OK */
        rspValue = AT_RSP_CONTINUE;
        break;

    default:
        break;
    }
    return rspValue;
}

void vatTest(void)
{
    AtCmdEntity atCmdInit[] = {
        {AT_CMD_DELAY "2000", 10, NULL},
        {"AT+WIMEI?" AT_CMD_END, 11, AtCmdCb_wimei},
        {"AT+ICCID" AT_CMD_END, 10, AtCmdCb_iccid},
        {"AT+CIMI" AT_CMD_END, 9, AtCmdCb_cimi},
    };
    iot_vat_push_cmd(atCmdInit, sizeof(atCmdInit) / sizeof(atCmdInit[0]));
}
