/* setjmp.h: ANSI 'C' (X3J11 Oct 88) library header, section 4.6 */

/* Copyright (C) ARM Ltd., 1999
 * All rights reserved
 * RCS $Revision: 102466 $
 * Checkin $Date: 2006-08-23 11:37:17 +0100 (Wed, 23 Aug 2006) $
 * Revising $Author: drodgman $
 */

/* Copyright (C) Codemist Ltd., 1988                            */
/* Copyright 1991 ARM Limited. All rights reserved.             */


/*
 * setjmp.h declares two functions and one type, for bypassing the normal
 * function call and return discipline (useful for dealing with unusual
 * conditions encountered in a low-level function of a program).
 */

#ifndef SETJMP_H
#define SETJMP_H
#ifdef __cplusplus
	  extern "C" {
#endif
	  
	  
#ifdef FPU
	  typedef unsigned long jmp_buf[22];
	  
#else
	  typedef unsigned long jmp_buf[12];
#endif
	  
	  int  setjmp (jmp_buf env);
	   
	  volatile void longjmp (jmp_buf env,  int value);
	   
#ifdef __cplusplus
	  }
#endif
#endif //SETJMP_H


/* end of setjmp.h */
