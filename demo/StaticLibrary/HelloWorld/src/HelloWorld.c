
#include <stdio.h>
#include "iot_debug.h"
#include "iot_os.h"
#include "HelloWorld.h"
void HelloFunc()
{
    for (int n = 0; n < 30; n++)
    {
        iot_debug_print("[LibHelloWorld]hello world %d", n);
        iot_os_sleep(1000);
    }
}
