#------------------------------------------------------------
# (C) Copyright [2006-2008] openat International Ltd.
# All Rights Reserved
#------------------------------------------------------------

#=========================================================================
# File Name      : newlib.mak
# Description    : Main make file for the /newlib package.
#
# Usage          : make [-s] -f newlib.mak OPT_FILE=<path>/<opt_file>
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

PACKAGE_NAME     = newlib
PACKAGE_BASE     = elua
PACKAGE_DEP_FILE = newlib_dep.mak
PACKAGE_PATH     = $(BUILD_ROOT)/$(PACKAGE_BASE)/$(PACKAGE_NAME)

# The path locations of source and include file directories.
PACKAGE_SRC_PATH    = $(PACKAGE_PATH)/src \
					  $(PACKAGE_PATH)/libc/src
PACKAGE_INC_PATHS   =
LOCAL_INC_PATHS     = $(PACKAGE_PATH)/include \
					  $(PACKAGE_PATH)/libc/include

#PACKAGE_SRC_FILES += $(wildcard *.c)
PACKAGE_SRC_FILES += devman.c \
                     genstd.c \
                     stubs.c  \
					 btea.c   \
                     errno.c  \
                     findfp.c \
                     locale.c \
                     malloc.c \
                     math.c   \
                     signal.c \
                     stdio.c  \
                     stdlib.c \
                     string.c \
                     time.c


# These are the tool flags specific to the newlib package only.
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

