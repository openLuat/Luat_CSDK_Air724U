#include "string.h"
#include "iot_debug.h"
#include "iot_gpio.h"
#include "iot_pmd.h"

#define gpio_print iot_debug_print
#define DEMO_GPIO_8 8
#define DEMO_GPIO_9 9

VOID demo_gpio_handle (E_OPENAT_DRV_EVT evt, 
                    E_AMOPENAT_GPIO_PORT gpioPort,
                unsigned char state)
{
    UINT8 status;

    // 判断是gpio中断
    if (OPENAT_DRV_EVT_GPIO_INT_IND == evt)
    {
        // 判断触发中断的管脚
        if (DEMO_GPIO_8 == gpioPort)
        {   
            // 触发电平的状态
            gpio_print("[gpio] input handle gpio %d, state %d", gpioPort, state);

            // 读当前gpio状态, 1:高电平 0:低电平
            iot_gpio_read(gpioPort, &status);
            gpio_print("[gpio] input handle gpio %d, status %d", gpioPort, state);
            
        }
    }
}


VOID demo_gpio_input(VOID)
{
    T_AMOPENAT_GPIO_CFG  input_cfg;
    BOOL err;
    
    memset(&input_cfg, 0, sizeof(T_AMOPENAT_GPIO_CFG));
    
    input_cfg.mode = OPENAT_GPIO_INPUT_INT; //配置输入中断
    input_cfg.param.defaultState = FALSE;    
    input_cfg.param.intCfg.debounce = 50;  //防抖50ms
    input_cfg.param.intCfg.intType = OPENAT_GPIO_INT_BOTH_EDGE; //中断触发方式双边沿
    input_cfg.param.intCfg.intCb = demo_gpio_handle; //中断处理函数
    err = iot_gpio_open(DEMO_GPIO_8, &input_cfg);

    if (!err)
        return;

    gpio_print("[gpio] set gpio8 input");
}

VOID demo_gpio_output(VOID)
{
    T_AMOPENAT_GPIO_CFG  output_cfg;
    BOOL err;
    
    memset(&output_cfg, 0, sizeof(T_AMOPENAT_GPIO_CFG));
    
    output_cfg.mode = OPENAT_GPIO_OUTPUT; //配置输出
    output_cfg.param.defaultState = FALSE; // 默认低电平

    err = iot_gpio_open(DEMO_GPIO_9, &output_cfg);

    if (!err)
        return;
        
    iot_gpio_set(DEMO_GPIO_9, TRUE); //设置为高电平
    
    gpio_print("[gpio] set gpio9 output");
}

VOID demo_gpio_init(VOID)
{
    demo_gpio_output(); //配置gpio8为输出
    demo_gpio_input(); //配置gpio9为输入
}

int appimg_enter(void *param)
{    
    gpio_print("[gpio] app_main");

    demo_gpio_init();

    return 0;
}

void appimg_exit(void)
{
    gpio_print("[gpio] appimg_exit");
}