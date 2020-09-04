/***************
	demo_hello
****************/

#include "iot_debug.h"
#include "iot_os.h"

#define fortest(fun, num, ms, ...) \
	for (char i = 0; i < num; i++) \
	{                              \
		fun(__VA_ARGS__);          \
		iot_os_sleep(ms);          \
	}

extern bool adcTest(E_AMOPENAT_ADC_CHANNEL channel);
extern void flashTest(UINT32 begain_addr, UINT32 end_addr);
extern void datetimeTest(void);
extern bool fsTest(char *file);
extern void networkTest(void);
extern bool ftpTest(void);
extern bool gsmlocTest(void);
extern void httpTest(void);
extern void pwdTest(void);
extern void RilTest(void);
extern void socketTest(char *mode);
extern void vatTest(void);
extern void zbarTest(void);
extern bool ttsTest(char *text, u32 len);

int appimg_enter(void *param)
{
	networkTest();
	extern bool networkstatus;
	while (networkstatus == FALSE)
		iot_os_sleep(500);
	//关闭看门狗，死机不会重启。默认打开
	iot_debug_set_fault_mode(OPENAT_FAULT_HANG);
	//打开调试信息，默认关闭
	iot_vat_send_cmd("AT^TRACECTRL=0,1,3\r\n", sizeof("AT^TRACECTRL=0,1,3\r\n"));
	iot_debug_print("[hello]appimg_enter");
	while (1)
	{
		fortest(zbarTest, 1, 200);
		fortest(adcTest, 1, 200, OPENAT_ADC_2);
		fortest(adcTest, 1, 200, OPENAT_ADC_3);
		fortest(flashTest, 1, 200, 0x320000, 0x330000);
		fortest(datetimeTest, 1, 200);
		fortest(fsTest, 1, 200, "/fs.test");

		//fortest(ftpTest, 0, 1000); //重复测试必死机，好像也是文件没有删除。

		//fortest(gsmlocTest, 1, 200);//长时间循环测试会导致死机
		fortest(httpTest, 1, 200); //有错误信息，我看不懂
		fortest(pwdTest, 1, 200);
		fortest(RilTest, 1, 200);

		fortest(socketTest, 1, 200, "DNS");
		fortest(socketTest, 1, 200, "UDP");
		fortest(socketTest, 1, 200, "TCP");
		//fortest(vatTest, 1, 200);//循环测试几次就会死机
		//fortest(ttsTest, 1, 200, "1", sizeof("1"));//几分钟就会死机
	}
	return 0;
}

void appimg_exit(void)
{
	iot_debug_print("[hello]appimg_exit");
}
