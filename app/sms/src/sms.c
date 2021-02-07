#include <stdio.h>
#include <string.h>
#include "at_process.h"
#include "at_tok.h"
#include "iot_debug.h"
#include "iot_os.h"
#include "gb2312_to_ucs2_table.h"
#include "ucs2_to_gb2312_table.h"
#include "am_openat_sms.h"

#define smslib_print 	iot_debug_print

typedef struct T_SmsInfo_TAG
{
	bool longsmsfg;
	int total;
	int isn;
	int cnt;
	char num[15];
	char data[400];
	char longdata[10][400];
	int longdatalen[10];
	char datetime[30];
}T_SmsInfo;

HANDLE demo_readsms_task;
extern bool g_IsSMSRdy;

static RecSmsHandlerCb g_recsmshandlecb = NULL;
static int isn = 0;
static T_SmsInfo SmsInfo = {0};
static char tnewsms[10] = {0};

static void check_smsready(void)
{
	while(!g_IsSMSRdy)
	{
		iot_os_sleep(1000);
	}
}

static void remove_tnewsms(void)
{
	int i;
	if(strlen(tnewsms)==0)
		return;
	
	for(i = 0; i< strlen(tnewsms); i++)
	{
		tnewsms[i] = tnewsms[i+1];
		
		if(tnewsms[i+2] == '\0')
			break;
	}
}

static void insert_tnewsms(int index)
{
	if(strlen(tnewsms)==10)
	{
		smslib_print("[smslib] tnewsms is full");
		return;
	}
	tnewsms[strlen(tnewsms)] = index; 
}

/* 0x4E00 <= ucs2 < 0xA000 */ 
static u16 get_ucs2_offset(u16 ucs2)
{
    u16   offset, page, tmp;
    u8    *mirror_ptr, ch;

    page = (ucs2>>8) - 0x4E;
    ucs2 &= 0xFF;

    tmp        = ucs2>>6; /* now 0 <= tmp < 4  */ 
    offset     = ucs2_index_table_4E00_9FFF_ext[page][tmp];  
    mirror_ptr = (u8*)&ucs2_mirror_4E00_9FFF_ext[page][tmp<<3]; /* [0, 8, 16, 24] */ 

    tmp = ucs2&0x3F; /* mod 64 */ 

    while(tmp >= 8)
    {
        offset += number_of_bit_1_ext[*mirror_ptr];
        mirror_ptr++;
        tmp -= 8;
    }

    ch = *mirror_ptr;
    if(ch&(0x1<<tmp))
    {   /* Ok , this ucs2 can be covert to GB2312. */ 
        while(tmp) 
        { 
            if(ch&0x1)
            offset++;
            ch>>=1;
            tmp--;
        }
        return offset;
    }

    return (u16)(-1);
}

static size_t iconv_ucs2_to_gb2312_endian_ext(char **_inbuf, size_t *inbytesleft, char **_outbuf, size_t *outbytesleft, int endian)
{
    u16 offset, gb2312 = 0xA1A1; 
    u16 ucs2;
    size_t gb_length = 0;
    u16 *ucs2buf = (u16*)*_inbuf;
    char *outbuf = (char *)*_outbuf;
    size_t inlen = *inbytesleft/2;
    size_t outlen = *outbytesleft;
    size_t ret = 1;

    while(inlen > 0)
    {
        if(gb_length+2 > outlen)
        {
            ret = 0;
            goto ucs2_to_gb2312_exit;
        }

        ucs2 = *ucs2buf++;

        if(endian == 1)
            ucs2 = (ucs2<<8)|(ucs2>>8);

        gb2312 = 0xA1A1;
        //End 7205
        if(0x80 > ucs2)
        {
            // can be convert to ASCII char
            *outbuf++ = (u8)ucs2;
            gb_length++;
        }
        else
        {
            if((0x4E00 <= ucs2) && (0xA000 > ucs2))
            {
                offset = get_ucs2_offset(ucs2);
                if((u16)(-1) != offset)
                {
                    gb2312 = ucs2_to_gb2312_table_ext[offset];
                }
            }
            else
            {
                u16 u16count = sizeof(tab_UCS2_to_GBK_ext)/4;
                u16 ui = 0;
                for(ui=0;ui<u16count;ui++)
                {
                    if(ucs2 == tab_UCS2_to_GBK_ext[ui][0])
                    {
                        gb2312 = tab_UCS2_to_GBK_ext[ui][1];
                    }
                }
                
            }
 
            *outbuf++ = (u8)(gb2312>>8);
            *outbuf++ = (u8)(gb2312);
            gb_length += 2;
        }
        
        inlen--;
    }

    if(inlen > 0)
    {
        ret = 0;
    }
	
	*outbytesleft = gb_length;
	return ret;

ucs2_to_gb2312_exit:
    *inbytesleft = inlen;
    *outbytesleft -= gb_length;

    return ret;
}


static size_t iconv_gb2312_to_ucs2_endian_ext(char **_inbuf, size_t *inbytesleft, char **_outbuf, size_t *outbytesleft, int endian)
{
    u16 offset,gb2312;
    char *gbbuf = *_inbuf;
    u16 *ucs2buf = (u16*)*_outbuf;
    u16 ucs2;
    size_t ucs2len = 0;
    size_t inlen = *inbytesleft;
    size_t outlen = *outbytesleft;
    size_t ret = 1;
    while(inlen > 0)
    {
        if(ucs2len+2 > outlen)
        {
            ret = 0;
            goto gb2312_to_ucs2_exit;
        }

        gb2312 = *gbbuf++;
		
        if(gb2312 < 0x80)
        {
            ucs2 = gb2312;
            inlen--;
        }
        else if(inlen >= 2)
        {
            gb2312 = (gb2312<<8) + ((*gbbuf++)&0x00ff);
            inlen -= 2;
            
            offset = ((gb2312>>8) - 0xA0)*94/*(0xFE-0xA1+1)*/ + ((gb2312&0x00ff) - 0xA1);
            ucs2 = gb2312_to_ucs2_table[offset];
        }
        else
        {
            break;
        }

        if(endian == 1)
            ucs2 = (ucs2<<8)|(ucs2>>8);

        *ucs2buf++ = ucs2;
        ucs2len += 2;
    }

    if(inlen > 0)
    {
        ret = 0;
    }
	*outbytesleft = ucs2len;
	return ret;
	
gb2312_to_ucs2_exit:
    *_inbuf = gbbuf;

    *inbytesleft = inlen;
    *outbytesleft -= ucs2len;

    return ret;
}

   
// 7-bit解码
// src: 源编码串指针
// dst: 目标字符串指针
// size: 源编码串长度
// 返回: 目标字符串长度
static int gsmDecode7bit(const void *src, void *dst, size_t size)
{
    const uint8_t *pSrc = (const uint8_t *)src;
    uint8_t *pDest = (uint8_t *)dst;
    uint16_t nSrc = 0;
    uint16_t nDst = 0;
    uint16_t nByte = 0;
    uint16_t nLeft = 0;

    while (nSrc < size)
    {
        *pDest = (((*pSrc << nByte) | nLeft) & 0x7f);
        nLeft = (*pSrc >> (7 - nByte));

        pDest++;
        nDst++;
        nByte++;

        if (nByte == 7)
        {
            *pDest = nLeft;
            pDest++;
            nDst++;
            nByte = 0;
            nLeft = 0;
        }
        pSrc++;
        nSrc++;
    }
    return nDst;
}

// 可打印字符串转换为字节数据
// 如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
// pSrc: 源字符串指针
// pDst: 目标数据指针
// nSrcLength: 源字符串长度
// 返回: 目标数据长度
static int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    for(int i=0; i<nSrcLength; i+=2)
    {
        // 输出高4位
        if(*pSrc>='0' && *pSrc<='9')
        {
            *pDst = (*pSrc - '0') << 4;
        }
        else
        {
            *pDst = (*pSrc - 'A' + 10) << 4;
        }
   
        pSrc++;
   
        // 输出低4位
        if(*pSrc>='0' && *pSrc<='9')
        {
            *pDst |= *pSrc - '0';
        }
        else
        {
             *pDst |= *pSrc - 'A' + 10;
        }
        pSrc++;
        pDst++;
    }
   
    // 返回目标数据长度
    return nSrcLength / 2;
}
   
// 字节数据转换为可打印字符串
// 如：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
// pSrc: 源数据指针
// pDst: 目标字符串指针
// nSrcLength: 源数据长度
// 返回: 目标字符串长度
static int gsmBytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    const char tab[]="0123456789ABCDEF";    // 0x0-0xf的字符查找表
   
    for(int i=0; i<nSrcLength; i++)
    {
        // 输出低4位
        *pDst++ = tab[*pSrc >> 4];
   
        // 输出高4位
        *pDst++ = tab[*pSrc & 0x0f];
   
        pSrc++;
    }
   
    // 输出字符串加个结束符
    *pDst = '\0';
   
    // 返回目标字符串长度
    return nSrcLength * 2;
}


// 正常顺序的字符串转换为两两颠倒的字符串，若长度为奇数，补'F'凑成偶数
// 如："8613851872468" --> "683158812764F8"
// pSrc: 源字符串指针
// pDst: 目标字符串指针
// nSrcLength: 源字符串长度
// 返回: 目标字符串长度
static int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
    int nDstLength;   // 目标字符串长度
    char ch;           // 用于保存一个字符
   
    // 复制串长度
    nDstLength = nSrcLength;
   
    // 两两颠倒
    for(int i=0; i<nSrcLength;i+=2)
    {
        ch = *pSrc++;        // 保存先出现的字符
        *pDst++ = *pSrc++;   // 复制后出现的字符
        *pDst++ = ch;        // 复制先出现的字符
    }
   
    // 源串长度是奇数吗？
    if(nSrcLength & 1)
    {
        *(pDst-2) = 'F';     // 补'F'
        nDstLength++;        // 目标串长度加1
    }
   
    // 输出字符串加个结束符
    *pDst = '\0';
   
    // 返回目标字符串长度
    return nDstLength;
}
   
// 两两颠倒的字符串转换为正常顺序的字符串
// 如："683158812764F8" --> "8613851872468"
// pSrc: 源字符串指针
// pDst: 目标字符串指针
// nSrcLength: 源字符串长度
// 返回: 目标字符串长度
static int gsmSerializeNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
    int nDstLength;   // 目标字符串长度
    char ch;          // 用于保存一个字符
   
    // 复制串长度
    nDstLength = nSrcLength;
   
    // 两两颠倒
    for(int i=0; i<nSrcLength;i+=2)
    {
        ch = *pSrc++;        // 保存先出现的字符
        *pDst++ = *pSrc++;   // 复制后出现的字符
        *pDst++ = ch;        // 复制先出现的字符
    }
	
	smslib_print("[smslib] gsmSerializeNumbers *(pDst-1): %c",*(pDst-1));	
    // 最后的字符是'F'吗？
    if(*(pDst-1) == 'F')
    {
        pDst--;
        nDstLength--;        // 目标字符串长度减1
    }
   
    // 输出字符串加个结束符
    *pDst = '\0';
   
    // 返回目标字符串长度
    return nDstLength;
}

static bool send_sms(char *pdu, int pdulen)
{
	ATResponse *p_response = NULL;
    bool result = FALSE;
	char* out;
	int err;
	char cmd[64];

	smslib_print("[smslib] send_sms start");

	memset(cmd, 0, 64);
	sprintf(cmd, "AT+CMGS=%d",pdulen);
	
    err = at_send_command_sms(cmd, pdu, "+CMGS:", &p_response);
    if (err < 0 || p_response->success == 0)
    {
        smslib_print("[smslib]at_send_command_singleline error %d",err);
        goto error;
    }
	
	return TRUE;
error:
	smslib_print("[smslib] send_sms end");
	if(p_response!=NULL)
	{
		at_response_free(p_response);
		p_response=NULL;
	}  
	return result;
}

bool gsmDecodePdu(char* pdu, int pdulen)
{
	bool longsms = FALSE;
	int offset = 0;
	int addlen = 0;
	char _addlen[3] = {0};
	char out[15] = {0};
	u8 flag = 0;
	u8 dcs = 0;
	char tz[15] = {0};
	int txtlen = 0;
	u8 fo = 0;
	char buff[400]  = {0};
	char *buf = OPENAT_malloc(400);
	char ppdu[400] = {0};
	char *data = OPENAT_malloc(400);
	int datalen = 160;
	int udhl = 0;
	int buflen = 0;
	int isn = 0;
	int total = 0;
	int idx = 0;
	
	if(pdu == NULL)
	{
		OPENAT_free(buf);
		OPENAT_free(data);
		return FALSE;
	}
	/*--PDU数据，不包括短信息中心号码*/
	strcpy(ppdu, pdu+((strlen(pdu)/2-pdulen)*2));
	smslib_print("[smslib]gsmDecodePdu ppdu: %s", ppdu);

	gsmString2Bytes(ppdu, buf, 2);
	/*--PDU短信首字节的高4位,第6位为数据报头标志位*/
	fo = buf[0];
	if(fo & 0x40)
	{
		longsms = TRUE;
		SmsInfo.longsmsfg =TRUE;
	}
	else
	{
		SmsInfo.longsmsfg =FALSE;
	}
	smslib_print("[smslib]gsmDecodePdu fo: %02x, longsms: %d", fo, longsms);

	gsmString2Bytes(ppdu+2, _addlen, 2);
	/*-回复地址数字个数*/
	addlen = _addlen[0];
	addlen = ((addlen%2==0)?(addlen):(addlen+1));
	offset = offset + addlen;
	
	smslib_print("[smslib]gsmDecodePdu ppdu[6]: %c, ppdu[7]: %c", ppdu[6], ppdu[7]);
	#if 1	
	if((ppdu[6] == '6') && (ppdu[7] == '8'))
	{
		strncpy(out, ppdu+8, addlen-2);
		addlen = addlen-2;
	}
	else
		strncpy(out, ppdu+6, addlen);
	smslib_print("[smslib]gsmDecodePdu 1 out: %s", out);
	gsmSerializeNumbers(out,SmsInfo.num,addlen);
	smslib_print("[smslib]gsmDecodePdu _addnum: %s, SmsInfo.num: %s", out, SmsInfo.num);	

	gsmString2Bytes(ppdu+offset+6, buf, 2);
	/*--协议标识 (TP-PID)*/
	flag = buf[0];
	smslib_print("[smslib]gsmDecodePdu flag: %d, offset: %d", flag, offset);
	offset = offset + 2;
	gsmString2Bytes(ppdu+offset+6, buf, 2);
	/*--用户信息编码方式 Dcs=8，表示短信存放的格式为UCS2编码*/
	dcs = buf[0];
	smslib_print("[smslib]gsmDecodePdu dcs: %d, offset: %d", dcs, offset);
	offset = offset + 2;
	strncpy(out, ppdu+offset+6, 14);
	smslib_print("[smslib]gsmDecodePdu out: %s", out);
	/*--时区7个字节*/
	gsmSerializeNumbers(out,tz,14);
	smslib_print("[smslib]gsmDecodePdu tz: %s, offset: %d", tz, offset);
	sprintf(SmsInfo.datetime, "%c%c/%c%c/%c%c,%c%c:%c%c:%c%c+%c%c",
						tz[0], tz[1], tz[2], tz[3], tz[4], tz[5], tz[6],
						tz[7], tz[8], tz[9], tz[10], tz[11], tz[12], tz[13]);
	
	offset = offset + 14;
	gsmString2Bytes(ppdu+offset+6, buf, 2);
	/*--短信文本长度*/
	txtlen = buf[0];
	smslib_print("[smslib]gsmDecodePdu txtlen: %d, offset: %d", txtlen, offset);
	offset = offset + 2;
	strcpy(data, ppdu+offset+6);
	/*--短信文本*/
	buflen = gsmString2Bytes(data, buff, strlen(data));
	smslib_print("[smslib]gsmDecodePdu data: %s, offset: %d", data, offset);
	if(longsms)
	{
		if(buff[2]  == 3)
		{
			isn = buff[3];
			total = buff[4];
			idx = buff[5];
			SmsInfo.isn = isn;
			SmsInfo.total = total;
			/*--去掉报头6个字节*/
			udhl = 7;
			memcpy(buf, buff, buflen);
		}
		else if(buff[2] == 4)
		{
			isn = (buff[3]<<8)|buff[4];
			total = buff[5];
			idx = buff[6];
			SmsInfo.isn = isn;
			SmsInfo.total = total;
			/*--去掉报头7个字节*/
			udhl = 8;
			memcpy(buf, buff, buflen);
		}
	}
	else
	{
		memcpy(buf, buff, buflen);
	}
	smslib_print("[smslib]gsmDecodePdu isn: %d, total: %d, idx: %d, udhl: %d", isn, total, idx, udhl);
	if(dcs == 0x00)/*--7bit encode*/
	{
		if(longsms)
		{
			memset(buf, 0, 400);
			datalen = gsmDecode7bit(buff, buf, buflen);
			memcpy(SmsInfo.longdata[idx-1], buf+udhl+1, datalen-udhl-1);
			SmsInfo.longdatalen[idx-1] = datalen-udhl-1;
			SmsInfo.cnt++;
		}
		else
		{
			gsmDecode7bit(buf, SmsInfo.data, buflen);
		}
	}
	else if(dcs == 0x04)/*--8bit encode*/
	{
		if(longsms)
		{
			memcpy(SmsInfo.longdata[idx-1], buf, buflen);
			SmsInfo.longdatalen[idx-1] = buflen;
			SmsInfo.cnt++;
		}
		else
		{
	 		memcpy(SmsInfo.data, buf, buflen);
		}
	}
	else
	{
		if(longsms)
		{
			memcpy(SmsInfo.longdata[idx-1],  buf+udhl+1, buflen-udhl-1);
			SmsInfo.longdatalen[idx-1] = buflen-udhl-1;
			SmsInfo.cnt++;
		}
		else
		{
			memcpy(SmsInfo.data, buf, buflen);
		}
	}	
	#endif
	OPENAT_free(buf);
	OPENAT_free(data);
	return TRUE;
}

static bool delete_sms(int index)
{
	int err;
  	ATResponse *p_response = NULL;
	char cmd[64];

	smslib_print("[smslib] send_sms start");

	memset(cmd, 0, 64);
	sprintf(cmd, "AT+CMGD=%d", index);
	err = at_send_command(cmd, &p_response);
	smslib_print("[smslib]CMGD error %d, success %d", err,(p_response?p_response->success:-1));
	if (err < 0 || p_response->success == 0){
		goto error;
	}
	
	return TRUE;
error:
	at_response_free(p_response);
	return FALSE;

}

static void longsmsmerge(int status)
{
	smslib_print("[smslib] longsmsmerge status: %d, SmsInfo.cnt: %d, SmsInfo.total:%d", status, SmsInfo.cnt, SmsInfo.total);
	if(SmsInfo.cnt != SmsInfo.total)
		return ;
	char* data = OPENAT_malloc(600);
	int datalen = 0;

	int i;
	for(i = 0; i < SmsInfo.total; i++)
	{
		memcpy(data+datalen, SmsInfo.longdata[i], SmsInfo.longdatalen[i]);
		datalen += SmsInfo.longdatalen[i];
		SmsInfo.longdatalen[i] = 0;
		memset(SmsInfo.longdata[i], 0, 400);
	}
	
	if(g_recsmshandlecb != NULL)
		g_recsmshandlecb(status, data, SmsInfo.num, SmsInfo.datetime);
	
	memset(SmsInfo.num, 0, 15);
	memset(SmsInfo.datetime, 0, 30);
	SmsInfo.cnt = 0;
	SmsInfo.total = 0;
	SmsInfo.longsmsfg = FALSE;
	OPENAT_free(data);

}

static void sms_read()
{
	if(strlen(tnewsms) > 0)
	{
		int err;
	  	ATResponse *p_response = NULL;
		char* out;
		char cmd[64];
		int pdulen = 0;
		char *pdu = OPENAT_malloc(500);
		memset(cmd, 0, 64);
		smslib_print("[smslib] read_sms tnewsms[0]: %d", tnewsms[0]);
		
		sprintf(cmd, "AT+CMGR=%d", tnewsms[0]);
		//+CMGR: 0,,25
		//0891683108501505F0040D91685112723869F20000121062519085230531D98C5603

		err = at_send_command_singleline(cmd, "+CMGR:", &p_response);
		if (err < 0 || p_response->success == 0){
			goto error;
		}
		char* line = p_response->p_intermediates->line;  
		char* line1 = p_response->p_intermediates->p_next->line; 
		
	    err = at_tok_start(&line);
	    if (err < 0)
	        goto error;
		err = at_tok_nextstr(&line, &out);
		if (err < 0)
			goto error;
		err = at_tok_nextstr(&line, &out);
		if (err < 0)
			goto error;
		err = at_tok_nextstr(&line, &out);
		if (err < 0)
			goto error;
		pdulen = atoi(out);
		strcpy(pdu, line1);
		smslib_print("[smslib] read_sms 0 pdulen: %d, pdu: %s", pdulen, pdu);
		
		err = gsmDecodePdu(pdu,pdulen);
		smslib_print("[smslib]read sms err: %d", err);
		#if 1
		if(!SmsInfo.longsmsfg)
		{
			if(g_recsmshandlecb != NULL)
				g_recsmshandlecb(err, SmsInfo.data, SmsInfo.num, SmsInfo.datetime);
			memset(SmsInfo.data, 0, 161);
			memset(SmsInfo.num, 0, 15);
			memset(SmsInfo.datetime, 0, 30);
		}
		else
		{
			/*长短信处理*/
			longsmsmerge(err);
		}
		#endif
		delete_sms(tnewsms[0]);
		OPENAT_free(pdu);
		remove_tnewsms();
		/*读取下一条短信*/
		sms_read();
		return;
	error:
		at_response_free(p_response);
		OPENAT_free(pdu);
		return;
	}
	iot_os_delete_task(demo_readsms_task);
}

static void _unsolSMSHandler(const int sms_index)
{
	smslib_print("[smslib]demo_unsolSMSHandler sms_index %d", sms_index);
	insert_tnewsms(sms_index);
	smslib_print("[smslib]demo_unsolSMSHandler strlen(tnewsms) %d", strlen(tnewsms));
	if(strlen(tnewsms) == 1)
		demo_readsms_task = iot_os_create_task(sms_read, NULL, 1024*6, 32, OPENAT_OS_CREATE_DEFAULT, "sms_read");
}

bool sms_init(void)
{
	/*检测等待SMS READY上报*/
	check_smsready();
	at_regSmsHanlerCb(_unsolSMSHandler);

	int err;
  	ATResponse *p_response = NULL;
	err = at_send_command("AT+CMGF=0", &p_response);
	smslib_print("[smslib]CMGF error %d, success %d", err,(p_response?p_response->success:-1));
	if (err < 0 || p_response->success == 0){
		goto error;
	}

	err = at_send_command("AT+CSMP=17,167,0,8", &p_response);
	smslib_print("[smslib]CSMP error %d, success %d", err,(p_response?p_response->success:-1));
	if (err < 0 || p_response->success == 0){
		goto error;
	}

	err = at_send_command("AT+CSCS=\"UCS2\"", &p_response);
	smslib_print("[smslib]CSMP error %d, success %d", err,(p_response?p_response->success:-1));
	if (err < 0 || p_response->success == 0){
		goto error;
	}

	err = at_send_command("AT+CPMS=\"SM\"", &p_response);
	smslib_print("[smslib]CSMP error %d, success %d", err,(p_response?p_response->success:-1));
	if (err < 0 || p_response->success == 0){
		goto error;
	}

	err = at_send_command("AT+CNMI=2,1,0,0", &p_response);
	smslib_print("[smslib]CNMI error %d, success %d", err,(p_response?p_response->success:-1));
	if (err < 0 || p_response->success == 0){
		goto error;
	}

	err = at_send_command("AT+CMGD=1,3", &p_response);
	smslib_print("[smslib]CNMI error %d, success %d", err,(p_response?p_response->success:-1));
	if (err < 0 || p_response->success == 0){
		goto error;
	}
	
	return TRUE;
error:
	at_response_free(p_response);
	return FALSE;
}

BOOL sms_send(char* num, char* data)
{
	char pnum[15] = {0};
	char ppnum[15] = {0};
	int pnumlen = 0;
	char pdatalen[4] = {0};
	int pducnt = 0;
	char* pdu = OPENAT_malloc(500);
	char* ppdu = NULL;
	int pdulen = 0;
	char udhi[17] = {0};
	int i;
	char numfix = 0x81;
	
	int datalen = strlen(data);
	int _pdatalen = (2*strlen(data));
	char* pdata = OPENAT_malloc(_pdatalen+1);
	memset(pdata, 0, (_pdatalen+1));
	char* ppdata = OPENAT_malloc((2*_pdatalen)+1);
	memset(ppdata, 0, (2*_pdatalen)+1);
	if(!iconv_gb2312_to_ucs2_endian_ext(&data, &datalen, &pdata, &_pdatalen,1))
	{
		smslib_print("sms ERROR");
		OPENAT_free(pdata);
		OPENAT_free(ppdata);
		OPENAT_free(pdu);
		return FALSE;
	}
	 
	int ppdatalen = gsmBytes2String(pdata, ppdata, _pdatalen);

	if(ppdatalen > 140)
	{
		pducnt = (_pdatalen + 133)/134 - 1;
		isn = (isn == 255) ? 0 : (isn + 1);
	}
	
	if(num[0] == '+')
	{
		numfix = 0x91;
		pnumlen = (strlen(num)-1);
		strncpy(pnum, &num[1], pnumlen);
	}
	else
	{
		pnumlen = strlen(num);
		strncpy(pnum, &num[0], pnumlen);
	}
	
	for(i = 0; i <= pducnt; i++)
	{
		memset(pdu, 0, 500);
		memset(ppnum, 0, 15);
		ppdu = pdu;
		if(pducnt > 0)
		{	
			char len_mul = 0x8C;
			if(i == pducnt)
				len_mul = _pdatalen-(pducnt*134) + 6;
			//udhi:6位协议头格式
			sprintf(udhi, "050003%02X%02X%02X", isn, pducnt+1, i+1);
			smslib_print("sms udhi:%s ", udhi);
			gsmInvertNumbers(pnum, ppnum, pnumlen);
			ppdu += sprintf(ppdu, "005110%02X%02X", pnumlen,numfix);
			strcat(ppdu, ppnum);
			ppdu += strlen(ppnum);
			ppdu += sprintf(ppdu, "000800%02X",len_mul);
			strcat(ppdu, udhi);
			ppdu += strlen(udhi);
			if(i == pducnt)
				strncat(ppdu, (ppdata + i*134*2), (ppdatalen - i*134*2));
			else
				strncat(ppdu, (ppdata + i*134*2), 134*2);
		}
		else
		{
			gsmInvertNumbers(pnum, ppnum, pnumlen);
			ppdu += sprintf(ppdu, "001110%02X%02X", pnumlen,numfix);
			strcat(ppdu, ppnum);
			ppdu += strlen(ppnum);
			ppdu += sprintf(ppdu, "000800%02X", _pdatalen);
			strcat(ppdu, ppdata);
		}
		pdulen = (strlen(pdu)/2) -1;
		smslib_print("sms pdu:%s ", pdu);
		smslib_print("sms pdu+250:%s ppdulen: %d", pdu+250, pdulen);
		if(!send_sms(pdu,pdulen))
		{
			return FALSE;
		}
	}
	OPENAT_free(pdata);
	OPENAT_free(ppdata);
	OPENAT_free(pdu);
	return TRUE;
}

void sms_setsmscb(RecSmsHandlerCb cb)
{
	g_recsmshandlecb = cb;
}


