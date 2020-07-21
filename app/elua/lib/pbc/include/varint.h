#ifndef PROTOBUF_C_VARINT_H
#define PROTOBUF_C_VARINT_H

//#include <stdint.h>


#ifndef __INT8_T__
typedef char        int8_t;
#define __INT8_T__
#endif

#ifndef __INT16_T__
typedef short       int16_t;
#define __INT16_T__
#endif

#ifndef __INT32_T__
typedef int         int32_t;
#define __INT32_T__
#endif

#ifndef __UINT8_T__
typedef unsigned char   uint8_t;
#define __UINT8_T__
#endif

#ifndef __UINT16_T__
typedef unsigned short  uint16_t;
#define __UINT16_T__
#endif


#ifndef __UINT32_T__
typedef unsigned int    uint32_t;
#define __UINT32_T__
#endif

#ifndef __UINT64_T__
typedef unsigned long long     uint64_t;
#define __UINT64_T__
#endif

#ifndef __INT64_T__
typedef signed long long     int64_t;
#define __INT64_T__
#endif



struct longlong {
	uint32_t low;
	uint32_t hi;
};

int _pbcV_encode32(uint32_t number, uint8_t buffer[10]);
int _pbcV_encode(uint64_t number, uint8_t buffer[10]);
int _pbcV_zigzag32(int32_t number, uint8_t buffer[10]);
int _pbcV_zigzag(int64_t number, uint8_t buffer[10]);

int _pbcV_decode(uint8_t buffer[10], struct longlong *result);
void _pbcV_dezigzag64(struct longlong *r);
void _pbcV_dezigzag32(struct longlong *r);

#endif
