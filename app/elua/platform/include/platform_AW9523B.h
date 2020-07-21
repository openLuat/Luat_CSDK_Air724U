
#ifndef _PLATFORM_AW9523B_H_
#define _PLATFORM_AW9523B_H_

// *****************************************************************************



#define I2C_NULL_SLAVE_ADDR         (0xffff)

int platform_AW9523B_display( u8 num1, u8 num2, u8 num3);

void platform_AW9523B_set_gpio( u8 pin_num, u8 value);

void platform_AW9523B_init(void);
#endif 
