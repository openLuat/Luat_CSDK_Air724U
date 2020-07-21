#------------------------------------------------------------
# (C) Copyright [2006-2008] openat International Ltd.
# All Rights Reserved
#------------------------------------------------------------

#=========================================================================
# File Name      : modules.mak
# Description    : Main make file for the /modules package.
#
# Usage          : make [-s] -f modules.mak OPT_FILE=<path>/<opt_file>
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

PACKAGE_NAME     = modules
PACKAGE_BASE     = elua
PACKAGE_DEP_FILE = modules_dep.mak
PACKAGE_PATH     = $(BUILD_ROOT)/$(PACKAGE_BASE)/$(PACKAGE_NAME)

# The path locations of source and include file directories.
PACKAGE_SRC_PATH    = $(PACKAGE_PATH)/src

PACKAGE_INC_PATHS   = $(PACKAGE_PATH)/src $(PACKAGE_PATH)/includ

#PACKAGE_SRC_FILES += $(wildcard *.c)
PACKAGE_SRC_FILES += adc.c           \
                     audio.c         \
                     bit.c           \
                     bitarray.c      \
                     cpu.c           \
                     disp.c          \
                     factory.c       \
                     gpio_i2c.c      \
                     gps.c           \
                     hrsensor.c      \
                     i2c.c           \
                     l_uart.c        \
                     lpack.c         \
                     luaiconv.c      \
                     lzlib.c         \
                     pio.c           \
                     pmd.c           \
                     pwm.c           \
                     qr_encode.c     \
                     rtos.c          \
                     tcpipsock.c     \
                     ttsply.c        \
                     watchdog.c      \
                     spi.c


# These are the tool flags specific to the modules package only.
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

