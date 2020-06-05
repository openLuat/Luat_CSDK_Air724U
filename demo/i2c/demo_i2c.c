#include "string.h"
#include "iot_debug.h"
#include "iot_i2c.h"

#define i2c_print 		iot_debug_print
#define DEMO_I2C_PORT  	OPENAT_I2C_1
#define I2CSLAVEADDR		0x3c

VOID demo_write_cmd(UINT8 val)
{
	UINT8 regAddr = 0x00;
	iot_i2c_write(DEMO_I2C_PORT, I2CSLAVEADDR, &regAddr, &val, 1);
}

VOID demo_write_data(UINT8 data)
{
	UINT8 regAddr = 0x40;
	iot_i2c_write(DEMO_I2C_PORT, I2CSLAVEADDR, &regAddr, &data, 1);
}

VOID demo_i2c_show(VOID)
{
	i2c_print("[i2c] enter demo_i2c_show");
	int i;
	UINT8 cmd[] = {0xAE, 0X00, 0x10, 0x40, 0x81, 0x7f, 0xA1, 0XA6, 0XA8, 63, 0XC8, 0XD3, 0X00, 0XD5, 0X80, 0XDA, 0X12, 0X8D, 0X14, 0X20, 0X01, 0XAF};
	UINT8 cmd2[] =	{0X21, 0X00, 0X7F};
	UINT8 cmd_G[] = {0x0, 0x0,0xf0, 0x7,0xf8, 0xf,0x8, 0x8,0x98, 0xf,0x90, 0x7,0x0, 0x0};
	UINT8 cmd_O[] = {0x0, 0x0,0xf0, 0x7,0xf8, 0xf,0x8, 0x8,0xf8, 0xf,0xf0, 0x7,0x0, 0x0};
	UINT8 cmd_[]  = {0x0, 0x0,0x0, 0x0,0xf8, 0xd,0xf8, 0xd,0x0, 0x0,0x0, 0x0,0x0, 0x0};
	
	demo_write_cmd(0xAF);
	demo_write_cmd(0xAF);
	demo_write_cmd(0xAF);
	demo_write_cmd(0xAF);
	for(i = 0; i < sizeof(cmd)/sizeof(cmd[0]); i++)
	{
		demo_write_cmd(cmd[i]);	
	}
	for(i = 0; i < sizeof(cmd2)/sizeof(cmd2[0]); i++)
	{
		demo_write_cmd(cmd2[i]);	
	}
	for(i = 0; i < 128*4; i++)
	{
		demo_write_data(0);	
		demo_write_data(0);
	}
	demo_write_cmd(0x22);
	demo_write_cmd(2);
	demo_write_cmd(3);

	demo_write_cmd(0x21);
	demo_write_cmd(43);
	demo_write_cmd(63);
	
	for(i = 0; i < sizeof(cmd_G)/sizeof(cmd_G[0]); i++)
	{
		demo_write_data(cmd_G[i]);
	}
	for(i = 0; i < sizeof(cmd_O)/sizeof(cmd_O[0]); i++)
	{
		demo_write_data(cmd_O[i]);
	}
	for(i = 0; i < sizeof(cmd_)/sizeof(cmd_[0]); i++)
	{
		demo_write_data(cmd_[i]);
	}
	UINT8 regAddr = 0x20;
	UINT8 data;
	iot_i2c_read(DEMO_I2C_PORT, I2CSLAVEADDR, &regAddr, &data, 1);
	i2c_print("[i2c] demo_i2c_show data %02x",data);
	i2c_print("[i2c] end demo_i2c_show");
}

VOID demo_i2c_open(VOID)
{
	i2c_print("[i2c] demo_i2c_open");
    BOOL err;
    T_AMOPENAT_I2C_PARAM i2cCfg;
    
    memset(&i2cCfg, 0, sizeof(T_AMOPENAT_I2C_PARAM));
	i2cCfg.freq = 100000;
	i2cCfg.regAddrBytes = 0;
	i2cCfg.noAck = FALSE;
	i2cCfg.noStop = FALSE;
	i2cCfg.i2cMessage = 0;

	err = iot_i2c_open(DEMO_I2C_PORT, &i2cCfg);
	iot_debug_print("[i2c] err %d", err);
}

VOID demo_i2c_close(VOID)
{
	iot_i2c_close(DEMO_I2C_PORT);
	i2c_print("[i2c] demo_i2c_close");
}

VOID demo_init(VOID)
{   
	demo_i2c_open();
	int i = 100;
	while(i--)
	{
		iot_os_sleep(3000);
		demo_i2c_show();
	}
}

int appimg_enter(void *param)
{    
    iot_os_sleep(1000);
	i2c_print("[i2c] app_main");
    demo_init();
	demo_i2c_close();
    return 0;
}

void appimg_exit(void)
{
    i2c_print("[i2c] appimg_exit");
}

