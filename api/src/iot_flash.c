#include "iot_flash.h"



/**��ȡflash���õĵ�ַ�ռ䣬���صĵ�ַ��������iot_flash_erase��iot_flash_write��iot_flash_read�Ƚӿڡ�
*@param		addrout:	���ؿ���flash��ַ
*@param		lenout:	���ؿ���flash���ȣ���λΪ�ֽ�
*@return	E_AMOPENAT_MEMD_ERR: 	�ɹ�:OPENAT_MEMD_ERR_NO, ����ʧ��
*@note      �ýӿڷ��صĵ�ַ��64KB���� ���صĵ�ַ�ռ���ݵ�ǰ�����С��ȷ����

**/
VOID iot_flash_getaddr(    
                    UINT32* addrout,
                    UINT32* lenout
               )
{
    if(addrout)
    {
        *addrout = 0xFFFFFFFF;
    }
    if(lenout)
    {
        *lenout = 0;
    }
}

/**flash�� 
*@param		startAddr:		��д��ַ 64K����
*@param		endAddr:		��д������ַ
*@return	E_AMOPENAT_MEMD_ERR: 	�ɹ�:OPENAT_MEMD_ERR_NO, ����ʧ��
**/
E_AMOPENAT_MEMD_ERR iot_flash_erase(             
                        UINT32 startAddr,
                        UINT32 endAddr
                   )
{
    return OPENAT_flash_erase(startAddr, endAddr);
}

/**flashд 
*@param		startAddr:		д��ַ 
*@param		size:		    д���ݴ�С
*@param		writenSize:		д�������ʹ�С
*@param		buf:		    д����ָ��
*@return	E_AMOPENAT_MEMD_ERR: 	�ɹ�:OPENAT_MEMD_ERR_NO, ����ʧ��
**/
E_AMOPENAT_MEMD_ERR iot_flash_write(             
                        UINT32 startAddr,
                        UINT32 size,
                        UINT32* writenSize,
                        CONST UINT8* buf
                   )
{
    return OPENAT_flash_write(startAddr, size, writenSize, buf);
}

/**flash��
*@param		startAddr:		����ַ 
*@param		size:		    �����ݴ�С
*@param		readSize:		���������ʹ�С
*@param		buf:		    ������ָ��
*@return	E_AMOPENAT_MEMD_ERR: 	�ɹ�:OPENAT_MEMD_ERR_NO, ����ʧ��
**/
E_AMOPENAT_MEMD_ERR iot_flash_read(               
                        UINT32 startAddr,
                        UINT32 size,
                        UINT32* readSize,
                        UINT8* buf
                   )
{
    return OPENAT_flash_read(startAddr, size, readSize, buf);
}
