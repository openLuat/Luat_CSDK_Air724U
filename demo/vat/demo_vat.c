#include "string.h"
#include "iot_debug.h"
#include "iot_vat.h"


extern int CsqValue;
static AtCmdRsp AtCmdCb_csq(u8* pRspStr)
{
	iot_debug_print("[vat]AtCmdCb_csq");
    AtCmdRsp  rspValue = AT_RSP_WAIT;
    u8 *rspStrTable[ ] = {"+CME ERROR","+CSQ: ", "OK"};
    s16  rspType = -1;
	u8 zero = '0';
    u8  i = 0;
    u8  *p = pRspStr + 2;
    for (i = 0; i < sizeof(rspStrTable) / sizeof(rspStrTable[0]); i++)
    {
        if (!strncmp(rspStrTable[i], p, strlen(rspStrTable[i])))
        {
            rspType = i;
            if (rspType == 1){
				if(p[strlen(rspStrTable[rspType])+1] == ',')
				{
			  		CsqValue = STR_TO_INT(p[strlen(rspStrTable[rspType])]);
				}else
				{
					CsqValue = STR_TO_INT(p[strlen(rspStrTable[rspType])])*10 + STR_TO_INT(p[strlen(rspStrTable[rspType])+1]);
				}
            }
            break;
        }
    }
    switch (rspType)
    {
        case 0:  /* ERROR */
        rspValue = AT_RSP_ERROR;
        break;

        case 1:  /* +CSQ */
		rspValue  = AT_RSP_WAIT;
        break;

		case 2:  /* OK */
		if(0 == CsqValue)
		{
			rspValue  = AT_RSP_STEP - 1;
		}
		else
		{
			iot_debug_print("[vat]csq: %d",CsqValue);
			rspValue  = AT_RSP_CONTINUE;

		}
        break;

        default:
        break;
    }
    return rspValue;
}


static AtCmdRsp AtCmdCb_wimei(u8* pRspStr)
{
	iot_debug_print("[vat]AtCmdCb_wimei");
    AtCmdRsp  rspValue = AT_RSP_WAIT;
    u8 *rspStrTable[ ] = {"+CME ERROR","+WIMEI:", "OK"};
    s16  rspType = -1;
    u8 imei[16] = {0};
    u8  i = 0;
    u8  *p = pRspStr + 2;
    for (i = 0; i < sizeof(rspStrTable) / sizeof(rspStrTable[0]); i++)
    {
        if (!strncmp(rspStrTable[i], p, strlen(rspStrTable[i])))
        {
            rspType = i;
            if (rspType == 1){
				strncpy(imei,p+strlen(rspStrTable[i]),15);
				iot_debug_print("[vat]imei: %s",imei);
            }
            break;
        }
    }
    switch (rspType)
    {
        case 0:  /* ERROR */
        rspValue = AT_RSP_ERROR;
        break;

        case 1:  /* +wimei */
        rspValue  = AT_RSP_WAIT;
        break;

		case 2:  /* OK */
        rspValue  = AT_RSP_CONTINUE;
        break;
        default:
        break;
    }
    return rspValue;
}

static AtCmdRsp AtCmdCb_iccid(u8* pRspStr)
{
	iot_debug_print("[vat]AtCmdCb_iccid");
    AtCmdRsp  rspValue = AT_RSP_WAIT;
    u8 *rspStrTable[ ] = {"ERROR","+ICCID:"};
    s16  rspType = -1;
    u8 iccid[20] = {0};
    u8  i = 0;
    u8  *p = pRspStr + 2;
    for (i = 0; i < sizeof(rspStrTable) / sizeof(rspStrTable[0]); i++)
    {
        if (!strncmp(rspStrTable[i], p, strlen(rspStrTable[i])))
        {
            rspType = i;
            if (rspType == 1){
				strncpy(iccid,p+strlen(rspStrTable[i]),20);
            }
            break;
        }
    }
    switch (rspType)
    {
        case 0:  /* ERROR */
        rspValue = AT_RSP_ERROR;
        break;

        case 1:  /* iccid */
			rspValue  = AT_RSP_WAIT;
        break;

		case 2:  /* OK */
        rspValue  = AT_RSP_CONTINUE;
        break;

        default:
        break;
    }
    return rspValue;
}

static AtCmdRsp AtCmdCb_cimi(u8* pRspStr)
{
	iot_debug_print("[vat]AtCmdCb_cimi");
    AtCmdRsp  rspValue = AT_RSP_WAIT;
    u8 *rspStrTable[ ] = {"+CME ERROR","460", "OK"};
    s16  rspType = -1;
    u8 imsi[20] = {0};
    u8  i = 0;
    u8  *p = pRspStr + 2;
    for (i = 0; i < sizeof(rspStrTable) / sizeof(rspStrTable[0]); i++)
    {
        if (!strncmp(rspStrTable[i], p, strlen(rspStrTable[i])))
        {
            rspType = i;
            if (rspType == 1){
				strncpy(imsi,p,15);
				iot_debug_print("[vat]imsi: %s",imsi);
            }
            break;
        }
    }

    switch (rspType)
    {
        case 0:  /* ERROR */
        rspValue = AT_RSP_ERROR;
        break;

        case 1:  /* imsi */
        rspValue  = AT_RSP_WAIT;
        break;

		case 2:  /* OK */
        rspValue  = AT_RSP_CONTINUE;
        break;

        default:
        break;
    }
    return rspValue;
}

VOID luat_ATCmdSend(VOID)
{
	BOOL result = FALSE;
	AtCmdEntity atCmdInit[]={
		{"AT"AT_CMD_END,4,NULL},
		{AT_CMD_DELAY"2000",10,NULL},
		{"AT+CSQ"AT_CMD_END,8,AtCmdCb_csq},
		{"AT+WIMEI?"AT_CMD_END,11,AtCmdCb_wimei},
		{"AT+ICCID"AT_CMD_END,10,AtCmdCb_iccid},
		{"AT+CIMI"AT_CMD_END,9,AtCmdCb_cimi},
	};
	result = iot_vat_send_cmd_push(atCmdInit,sizeof(atCmdInit) / sizeof(atCmdInit[0]));
    return result;
}

int appimg_enter(void *param)
{    
    iot_debug_print("[vat] app_main");
	luat_ATCmdSend();

    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[vat] appimg_exit");
}

