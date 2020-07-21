
#ifndef __FEATURES_H__
#define __FEATURES_H__

/* Macro to test version of GCC.  Returns 0 for non-GCC or too old GCC. */
#ifndef __GNUC_PREREQ
# if defined __GNUC__ && defined __GNUC_MINOR__
#  define __GNUC_PREREQ(maj, min) \
	((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
# else
#  define __GNUC_PREREQ(maj, min) 0
# endif
#endif /* __GNUC_PREREQ */
/* Version with trailing underscores for BSD compatibility. */
#define	__GNUC_PREREQ__(ma, mi)	__GNUC_PREREQ(ma, mi)

#endif

