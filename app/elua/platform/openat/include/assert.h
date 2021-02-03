/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    assert.h
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2012/9/15
 *
 * Description:
 * 
 **************************************************************************/
#ifndef __ASSERT_H__
#define __ASSERT_H__

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


#define ASSERT(cOND) {if (!(cOND)) {platform_assert(__FILE__, (short)__LINE__);}}

#define assert(x) ASSERT(x)

void platform_assert(const char *func, int line);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__ASSERT_H__
