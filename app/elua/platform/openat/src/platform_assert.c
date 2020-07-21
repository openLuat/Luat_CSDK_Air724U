#include "am_openat.h"

void platform_assert(const char *func, int line)
{
  IVTBL(assert)(FALSE, func, line);

}

