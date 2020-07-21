/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2012/10/8
 *
 * Description:
 * 
 **************************************************************************/
#if 1
#include "assert.h"

#include "am_openat.h"
#include "lplatform.h"
#include "platform_malloc.h"
#include "platform_conf.h"
#include "common.h"

#if defined(BUILD_LUA_INT_HANDLERS) || defined(BUILD_C_INT_HANDLERS)
#define BUILD_INT_HANDLERS

#ifndef INT_TMR_MATCH
#define INT_TMR_MATCH ELUA_INT_INVALID_INTERRUPT
#endif

extern const elua_int_descriptor elua_int_table[INT_ELUA_LAST];

#endif // #if defined( BUILD_LUA_INT_HANDLERS ) || defined( BUILD_C_INT_HANDLERS )

static unsigned char luaConsolePort = 0;

int platform_init(void)
{
    cmn_platform_init();

    return PLATFORM_OK;
}

// ****************************************************************************
// Timer

void platform_s_timer_delay(unsigned id, u32 delay_us)
{
    ASSERT(0);
}

u32 platform_s_timer_op(unsigned id, int op, u32 data)
{
    u32 res = 0;

    switch (op)
    {
    case PLATFORM_TIMER_OP_START:
    case PLATFORM_TIMER_OP_READ:
    case PLATFORM_TIMER_OP_GET_MAX_DELAY:
    case PLATFORM_TIMER_OP_GET_MIN_DELAY:
    case PLATFORM_TIMER_OP_SET_CLOCK:
    case PLATFORM_TIMER_OP_GET_CLOCK:
        break;
    }
    return res;
}

// ****************************************************************************
// PIO functions

/*+\NEW\liweiqiang\2013.4.5\增加lua gpio 中断配置*/
static void GpioIntCallback(E_OPENAT_DRV_EVT evt, unsigned int gpioNum, unsigned char state)
{
    PlatformMsgData msgData;

    if (state)
        msgData.interruptData.id = INT_GPIO_POSEDGE;
    else
        msgData.interruptData.id = INT_GPIO_NEGEDGE;

    msgData.interruptData.resnum = gpioNum;

    platform_rtos_send(MSG_ID_RTOS_INT, &msgData);
}
/*-\NEW\liweiqiang\2013.4.5\增加lua gpio 中断配置*/

#define TOTAL_GPIO_PIN_COUNT 56

static unsigned int sDebounce = 10;

void platform_pio_set_debounce(unsigned int debounce)
{
    sDebounce = debounce;
}

/*+\NEW\zhuwangbin\2020.6.7\通过gpio设置方波*/
#ifdef AM_LUA_POC_SUPPORT
bool platform_gpioPulse(unsigned port, unsigned int time_us, unsigned int count)
{

    return openat_gpioPulse((E_AMOPENAT_GPIO_PORT)port, time_us, count);
}
#endif
/*-\NEW\zhuwangbin\2020.6.7\通过gpio设置方波*/

pio_type platform_pio_op(unsigned port_group_id, pio_type pinmask, int op)
{
    pio_type retval = 1;
    int pin_index;
    T_AMOPENAT_GPIO_CFG cfg;
    E_AMOPENAT_GPIO_PORT realGpio;

    for (pin_index = 0; pin_index < 32; pin_index++)
    {
        realGpio = (E_AMOPENAT_GPIO_PORT)((port_group_id << 5) + pin_index);

        if ((pinmask & (1 << pin_index)) != 0)
        {
            switch (op)
            {
            /*+\NEW\liweiqiang\2013.4.5\增加lua gpio 中断配置*/
            case PLATFORM_IO_PIN_DIR_INT:
                cfg.mode = OPENAT_GPIO_INPUT_INT;
                /*+\NEW\RUFEI\2015.6.26\Modified interrupt handle*/
                cfg.param.intCfg.debounce = sDebounce;
                /*-\NEW\RUFEI\2015.6.26\Modified interrupt handle*/
                cfg.param.intCfg.intType = OPENAT_GPIO_INT_BOTH_EDGE;
                cfg.param.intCfg.intCb = (OPENAT_GPIO_EVT_HANDLE)GpioIntCallback;
                retval = IVTBL(config_gpio)(realGpio, &cfg);
                break;
                /*-\NEW\liweiqiang\2013.4.5\增加lua gpio 中断配置*/

            case PLATFORM_IO_PIN_DIR_INPUT:
                cfg.mode = OPENAT_GPIO_INPUT;
                retval = IVTBL(config_gpio)(realGpio, &cfg);
                break;

            case PLATFORM_IO_PIN_DIR_OUTPUT:
            case PLATFORM_IO_PIN_DIR_OUTPUT1:
                cfg.mode = OPENAT_GPIO_OUTPUT;
                cfg.param.defaultState = op - PLATFORM_IO_PIN_DIR_OUTPUT;
                retval = IVTBL(config_gpio)(realGpio, &cfg);
                break;

            case PLATFORM_IO_PIN_SET:
                retval = IVTBL(set_gpio)(realGpio, 1);
                break;

            case PLATFORM_IO_PIN_CLEAR:
                retval = IVTBL(set_gpio)(realGpio, 0);
                break;

            case PLATFORM_IO_PIN_GET:
            {
                UINT8 gpioValue = 0xff;
                retval = IVTBL(read_gpio)(realGpio, &gpioValue);
                // 对于读取操作 一次只能读取一个pin
                return retval == TRUE ? gpioValue : 0xff;
                break;
            }

                /*+\NEW\liweiqiang\2013.4.11\增加pio.pin.close接口*/
            case PLATFORM_IO_PIN_CLOSE:
                retval = IVTBL(close_gpio)(realGpio);
                break;
                /*-\NEW\liweiqiang\2013.4.11\增加pio.pin.close接口*/
            /*+\NEW\lijiaodi\2018.08.10\添加管脚上下拉接口*/
            case PLATFORM_IO_PIN_PULLUP:
                retval = OPENAT_pin_set_pull(realGpio, 1);
                break;
            case PLATFORM_IO_PIN_PULLDOWN:
                retval = OPENAT_pin_set_pull(realGpio, 2);
                break;
            case PLATFORM_IO_PIN_NOPULL:
                retval = OPENAT_pin_set_pull(realGpio, 0);
                break;
            /*-\NEW\lijiaodi\2018.08.10\添加管脚上下拉接口*/
            // not support
            case PLATFORM_IO_PORT_DIR_INPUT:
            case PLATFORM_IO_PORT_DIR_OUTPUT:
            case PLATFORM_IO_PORT_SET_VALUE:
            case PLATFORM_IO_PORT_GET_VALUE:

            default:
                retval = 0;
                break;
            }
        }
    }

    return retval;
}

E_AMOPENAT_GPIO_PORT platform_pio_get_gpio_port(int port_pin)
{
    int port, pin;

    port = PLATFORM_IO_GET_PORT(port_pin);
    pin = PLATFORM_IO_GET_PIN(port_pin);

    return (E_AMOPENAT_GPIO_PORT)((port << 5) + pin);
}

// ****************************************************************************
int platform_cpu_set_global_interrupts(int status)
{
    return 0;
}

int platform_cpu_get_global_interrupts()
{
    return 0;
}

/*+\NEW\liweiqiang\2013.7.1\作长时间运算时自动调节主频加快运算速度*/
void platform_sys_set_max_freq(void)
{
}

void platform_sys_set_min_freq(void)
{
}
/*-\NEW\liweiqiang\2013.7.1\作长时间运算时自动调节主频加快运算速度*/

//console
void platform_set_console_port(unsigned char id)
{
    luaConsolePort = id;
}

unsigned char platform_get_console_port(void)
{
    return luaConsolePort;
}

/*+\NEW\liweiqiang\2013.6.6\增加adc库*/
// adc
int platform_adc_exists(unsigned id)
{
    return id < OPENAT_ADC_QTY;
}
/*+\NEW\RUFEI\2015.8.27\Add adc fuction*/
int platform_adc_open(unsigned id, unsigned mode)
{
    //return PLATFORM_OK;
    /*+\BUG\wangyuan\2020.06.30\lua版本编译不过*/
    return IVTBL(InitADC)(id, mode) ? PLATFORM_OK : PLATFORM_ERR;
    /*-\BUG\wangyuan\2020.06.30\lua版本编译不过*/
}

int platform_adc_close(unsigned id)
{
    return IVTBL(CloseADC)(id) ? PLATFORM_OK : PLATFORM_ERR;
}

int platform_adc_read(unsigned id, int *adc, int *volt)
{
    u16 adcVal = 0xFFFF;
    u16 voltage = 0xffff;
    BOOL ret;

    ret = IVTBL(ReadADC)(id, &adcVal, &voltage);

    *adc = voltage / 3;
    *volt = voltage;
    return ret ? PLATFORM_OK : PLATFORM_ERR;
}
/*-\NEW\RUFEI\2015.8.27\Add adc fuction*/
/*-\NEW\liweiqiang\2013.6.6\增加adc库*/

/*+\bug\wj\2020.4.30\lua添加pwm接口*/
/*+\NEW\RUFEI\2015.9.8\Add pwm function */
int platform_pwm_open(unsigned id)
{
    return OPENAT_pwm_open((E_AMOPENAT_PWM_PORT)id) ? PLATFORM_OK : PLATFORM_ERR;
}

int platform_pwm_close(unsigned id)
{
    return OPENAT_pwm_close((E_AMOPENAT_PWM_PORT)id) ? PLATFORM_OK : PLATFORM_ERR;
}

int platform_pwm_set(unsigned id, int param0, int param1)
{
    T_AMOPENAT_PWM_CFG pwmcfg;

    memset(&pwmcfg, 0, sizeof(T_AMOPENAT_PWM_CFG));
    switch (id)
    {
    case OPENAT_PWM_PWT_OUT:
    {
        pwmcfg.port = OPENAT_PWM_PWT_OUT;
        pwmcfg.cfg.pwt.freq = param0;
        pwmcfg.cfg.pwt.level = param1;
    }
    break;
    case OPENAT_PWM_LPG_OUT:

    {
        pwmcfg.port = OPENAT_PWM_LPG_OUT;
        pwmcfg.cfg.lpg.period = (E_OPENAT_PWM_LPG_PERIOD)param0;
        pwmcfg.cfg.lpg.onTime = (E_OPENAT_PWM_LPG_ON)param1;
    }
    break;
    case OPENAT_PWM_PWL_OUT0:
    {
        pwmcfg.port = OPENAT_PWM_PWL_OUT0;
        pwmcfg.cfg.pwl.freq = param0;
        pwmcfg.cfg.pwl.level = param1;
    }
    break;
    case OPENAT_PWM_PWL_OUT1:
    {
        pwmcfg.port = OPENAT_PWM_PWL_OUT0;
        pwmcfg.cfg.pwt.freq = param0;
        pwmcfg.cfg.pwt.level = param1;
    }
    break;
    default:
        return PLATFORM_ERR;
    }
    pwmcfg.port = (E_AMOPENAT_PWM_PORT)id;
    return OPENAT_pwm_set(&pwmcfg);
}
/*-\bug\wj\2020.4.30\lua添加pwm接口*/
/*-\NEW\RUFEI\2015.9.8\Add pwm function */
#endif
