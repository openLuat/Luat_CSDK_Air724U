

#include "iot_adc.h"

bool adcTest(E_AMOPENAT_ADC_CHANNEL channel)
{
    BOOL err = iot_adc_init(channel, OPENAT_ADC_MODE_MAX);
    if (!err)
    {
        iot_debug_print("[coreTest-False-adc] : ADC%d Init FALSE", channel);
        return FALSE;
    }
    UINT32 adcValue = 0, voltage = 0;
    err = iot_adc_read(channel, &adcValue, &voltage);
    if (!err)
    {
        iot_debug_print("[coreTest-False-adc] : ADC%d read FALSE", channel);
        return FALSE;
    }
    iot_debug_print("[coreTest-adc] : ADC%d read adcValue:%d,voltage:%d", channel, adcValue, voltage);
    return TRUE;
}