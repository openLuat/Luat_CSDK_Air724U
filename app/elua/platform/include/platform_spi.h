typedef enum PlatformSpiIdTag
{
    PLATFORM_SPI_1,
	PLATFORM_SPI_2,
    PLATFORM_SPI_QTY,
}PlatformSpiId;


s32 platform_spi_setup( u32 id, u32 clock, BOOL cpol, BOOL cpha, unsigned databits, BOOL fullduplex, BOOL withCS);


s32 platform_spi_send(u32 id, u8* data, int len);


s32 platform_spi_recv(u32 id, u8** data, int len);


s32 platform_spi_send_recv(u32 id, u8* sendData, u8** recvData, int len);

s32 platform_spi_close( u32 id );

