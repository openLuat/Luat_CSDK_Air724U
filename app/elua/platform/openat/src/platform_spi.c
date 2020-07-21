
#include <string.h>

#include "malloc.h"

#include "assert.h"
#include "lplatform.h"
#include "platform_spi.h"
#include "am_openat.h"


s32 platform_spi_setup( u32 id, u32 clock, BOOL cpol, BOOL cpha, unsigned databits, BOOL fullduplex, BOOL withCS)
{
    T_AMOPENAT_SPI_PARAM cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.clock = clock;
    cfg.cpha = cpha;
    cfg.cpol = cpol;
    cfg.dataBits = databits;
    cfg.fullDuplex = fullduplex;
    cfg.withCS = withCS;
    //IVTBL(send_event)(0x8b18b1);
    return IVTBL(OpenSPI)((E_AMOPENAT_SPI_PORT)id, &cfg);
}


s32 platform_spi_send(u32 id, u8* data, int len)
{
    u8* rev_temp;
    rev_temp = malloc(len);
    if(rev_temp == NULL || data == NULL)
    {
        return -1;
    }
    memset(rev_temp, 0, len);
	/*+\BUG1029\rww\2020.1.14\spi csw_mem 死机*/
	s32 rwlen = IVTBL(RwSPI)((E_AMOPENAT_SPI_PORT)id, data, rev_temp, len);
	free(rev_temp);
    return rwlen;
	/*-\BUG1029\rww\2020.1.14\spi csw_mem 死机*/
    //return IVTBL(WriteSPI)((E_AMOPENAT_SPI_PORT)id, data, len, NULL);
}


s32 platform_spi_recv(u32 id, u8** data, int len)
{
    *data = malloc(len);
    if(*data == NULL)
    {
        return -1;
    }
    u8* send;
    send = malloc(len);
    if(send == NULL)
    {
    /*+\BUG1029\rww\2020.1.14\spi csw_mem 死机*/
    	free(*data);
        return -1;
    }
    memset(send, 0, len);
	s32 rwlen = IVTBL(RwSPI)((E_AMOPENAT_SPI_PORT)id, send, *data, len);
	free(send);
    return rwlen;
	/*-\BUG1029\rww\2020.1.14\spi csw_mem 死机*/
    //return IVTBL(ReadSPI)((E_AMOPENAT_SPI_PORT)id, *data, len);
}

s32 platform_spi_send_recv(u32 id, u8* sendData, u8** recvData, int len)
{
    *recvData = malloc(len);
    return IVTBL(RwSPI)((E_AMOPENAT_SPI_PORT)id, sendData, *recvData, len);
}

s32 platform_spi_close( u32 id )
{
    return IVTBL(CloseSPI)((E_AMOPENAT_SPI_PORT)id);
}


