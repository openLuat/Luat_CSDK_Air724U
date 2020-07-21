#------------------------------------------------------------
# (C) Copyright [2006-2008] openat International Ltd.
# All Rights Reserved
#------------------------------------------------------------

#=========================================================================
# File Name      : openat.mak
# Description    : Main make file for the /openat package.
#
# Usage          : make [-s] -f openat.mak OPT_FILE=<path>/<opt_file>
#
# Notes          : The options file defines macro values defined
#                  by the environment, target, and groups. It
#                  must be included for proper package building.
#
# Copyright (c) 2002 Intel Corporation. All Rights Reserved
#=========================================================================

# Package build options

include $(OPT_FILE)

# Package Makefile information

GEN_PACK_MAKEFILE = $(BUILD_ROOT)/env/$(HOST)/build/package.mak

# Define Package ---------------------------------------

PACKAGE_NAME     = openat
PACKAGE_BASE     = luat
PACKAGE_DEP_FILE = openat_dep.mak
PACKAGE_PATH     = $(BUILD_ROOT)/$(PACKAGE_BASE)/elua/platform/$(PACKAGE_NAME)

# The path locations of source and include file directories.
PACKAGE_SRC_PATH    = $(PACKAGE_PATH)/src

PACKAGE_INC_PATHS   = $(PACKAGE_PATH)/src $(PACKAGE_PATH)/include

#PACKAGE_SRC_FILES += $(wildcard *.c)
PACKAGE_SRC_FILES += ctype.c                    \
                     cycle_queue.c              \
                     libc_time.c                \
                     platform.c                 \
                     platform_assert.c          \
                     platform_audio.c           \
                     platform_AW9523B.c         \
                     platform_factory.c         \
                     platform_fs.c              \
                     platform_gps.c             \
                     platform_hrsensor.c        \
                     platform_i2c.c             \
                     platform_int.c             \
                     platform_lcd.c             \
                     platform_main.c            \
                     platform_malloc.c          \
                     platform_pmd.c             \
                     platform_rtos.c            \
                     platform_SLI3108.c         \
                     platform_socket.c          \
                     platform_stdio.c           \
                     platform_sys.c             \
                     platform_tp.c              \
                     platform_ttsply.c          \
                     platform_uart.c            \
                     platform_watchdog.c        \
                     platform_spi.c


# These are the tool flags specific to the openat package only.
# The environment, target, and group also set flags.

PACKAGE_CFLAGS   =
PACKAGE_DFLAGS   =
PACKAGE_ARFLAGS  =

#
# Allow this definition to enable CCCR configurations
# followed by an infinite loop. (Raviv 20/12/05)
#
#PACKAGE_ASMFLAGS += -predefine "ENABLE_L1_LOOP SETL {TRUE}"
#



# Include the Standard Package Make File ---------------
include $(GEN_PACK_MAKEFILE)

# Include the Make Dependency File ---------------------
# This must be the last line in the file
include $(PACKAGE_DEP_FILE)

