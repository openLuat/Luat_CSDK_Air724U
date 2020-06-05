///*
// * These functions have been extracted from PDPCLIB (Public Domain Project
// * C Library) - See http://pdos.sourceforge.net/.
// */
//
///* rand() has been modified to return values in the correct range,
// * 0...0x7fff, by anding the result with 0x7fff and not 0x8fff.
// */
//
//#include "stdlib.h"
//#include "iot_debug.h"
//
//static unsigned long myseed = 1;
//
//void srand(unsigned int seed)
//{
//
//	iot_debug_print("ener srand ");
//    myseed = seed;
//    return;
//}
//
//int rand(void)
//{
//    int ret;
//
//    myseed = myseed * 1103515245UL + 12345;
//    ret = (int)((myseed >> 16) & 0x7fff);
//    return (ret);
//}
//
//void srand_xor(unsigned int value)
//{
//    myseed ^= value;
//    // Pump the bits around a bit.
//    (void) rand();
//}
