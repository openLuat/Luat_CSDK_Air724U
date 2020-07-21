
#ifndef __CRC_H__
#define __CRC_H__

//u16 calcCRC16(const u8 *data, u32 length);
u32 calcCRC32(const u8* buf, u32 len);
u8 calcCRC8(const u8 *buf, u32 len);
u16 calcCRC16(const u8 *data, const char *cmd, int length, u16 poly, u16 initial, u16 finally, BOOL bInReverse, BOOL bOutReverse);
#endif
