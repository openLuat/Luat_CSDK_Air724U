#include "string.h"
#include "iot_debug.h"
#include "iot_flash.h"

#define flash_print iot_debug_print

static UINT32 s_demo_flash_begain_addr, s_demo_flash_end_addr;

#define DEMO_FLASH_BEGIN_ADDR (s_demo_flash_begain_addr)
#define DEMO_FLASH_END_ADDR (s_demo_flash_end_addr)

bool demo_flash_read(VOID)
{
    UINT8 read_buff[64];
    UINT32 read_len;
    E_AMOPENAT_MEMD_ERR errCode;

    errCode = iot_flash_read(DEMO_FLASH_BEGIN_ADDR, sizeof(read_buff), &read_len, read_buff);

    if (OPENAT_MEMD_ERR_NO != errCode)
    {
        flash_print("[coreTest-False-flash] iot_flash_read false,err:%d", errCode);
        return FALSE;
    }
    flash_print("[coreTest-flash] : read_len %x, read_buff %s", read_len, read_buff);
    return TRUE;
}

VOID demo_flash_write(VOID)
{
    UINT8 write_buff[64] = {0};
    UINT32 write_len;
    E_AMOPENAT_MEMD_ERR errCode;

    memcpy(write_buff, "flash hello world", strlen("flash hello world"));
    errCode = iot_flash_write(DEMO_FLASH_BEGIN_ADDR, sizeof(write_buff), &write_len, write_buff);
    if (OPENAT_MEMD_ERR_NO != errCode)
    {
        flash_print("[coreTest-False-flash] iot_flash_write false,err:%d", errCode);
        return FALSE;
    }
    flash_print("[coreTest-flash] : write_len %x, write_buff %s", write_len, write_buff);
    return TRUE;
}

VOID demo_flash_erase(VOID)
{
    E_AMOPENAT_MEMD_ERR errCode;
    flash_print("[coreTest-flash] : erase {%x,%x}", DEMO_FLASH_BEGIN_ADDR, DEMO_FLASH_END_ADDR);
    errCode = iot_flash_erase(DEMO_FLASH_BEGIN_ADDR, DEMO_FLASH_END_ADDR);
    if (OPENAT_MEMD_ERR_NO != errCode)
    {
        flash_print("[coreTest-False-flash] : erase {%x,%x} error %d", DEMO_FLASH_BEGIN_ADDR, DEMO_FLASH_END_ADDR, errCode);
        return FALSE;
    }
    return TRUE;
}
VOID demo_flash_getaddr(VOID)
{
    UINT32 addrout = 0, lenout = 0;
    iot_flash_getaddr(&addrout, &lenout);
    flash_print("[coreTest-flash] : getaddr {%x,%x}", addrout, lenout);
    return TRUE;
}
void flashTest(UINT32 begain_addr, UINT32 end_addr)
{
    s_demo_flash_begain_addr = begain_addr; //0x60320000;
    s_demo_flash_end_addr = end_addr;       //0x60330000
    demo_flash_getaddr();                   //获取可用空间
    demo_flash_erase();                     // 擦flash
    demo_flash_write();                     // 写flash
    demo_flash_read();                      // 读flash
}
