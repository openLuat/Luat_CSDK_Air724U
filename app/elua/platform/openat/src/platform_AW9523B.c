#ifdef AM_AW9523B_GPIO_CHIP_SUPPORT

#include <string.h>

#include "lplatform.h"
#include "platform_conf.h"
#include "am_openat.h"

void platform_AW9523B_display( u8 num1, u8 num2, u8 num3)
{

    IVTBL(AW9523B_display)(num1,  num2,  num3);
}

#if 0
int platform_gpio_i2c_recv_data(u16 slave_addr, const u8 *pRegAddr, u8 *buf, u32 len  )
{
    // 如果从地址为空,则返回传输失败
    if(slave_addr == I2C_NULL_SLAVE_ADDR){
        return 0;
    }
    
    return IVTBL(cust_i2c_receive)(slave_addr, *pRegAddr, buf, len);
}

#endif

void platform_AW9523B_set_gpio( u8 pin_num, u8 value)
{
    IVTBL(AW9523B_set_gpio)( pin_num, value);
}

void platform_AW9523B_init(void)
{
    IVTBL(AW9523B_init)();
}

#endif
