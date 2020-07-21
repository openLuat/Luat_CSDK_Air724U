
#ifndef _BTEA_H_
#define _BTEA_H_

#ifdef __cplusplus
extern "C" {
#endif 
#define uint32_t unsigned int

void btea(uint32_t *v, int n, uint32_t const key[4]);

#ifdef __cplusplus
}
#endif
#endif

