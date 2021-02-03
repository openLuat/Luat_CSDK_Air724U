#------------------------------------------------------------
# (C) Copyright [2006-2008] Marvell International Ltd.
# All Rights Reserved
#------------------------------------------------------------

#=========================================================================
# File Name      : iconv.mak
# Description    : Main make file for the /lib package.
#
# Usage          : make [-s] -f lib.mak OPT_FILE=<path>/<opt_file>
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

PACKAGE_NAME     = lib
PACKAGE_BASE     = elua
PACKAGE_DEP_FILE = lib_dep.mak
PACKAGE_PATH     = $(BUILD_ROOT)/$(PACKAGE_BASE)/$(PACKAGE_NAME)

# The path locations of source and include file directories.
PACKAGE_SRC_PATH    = $(PACKAGE_PATH)/iconv/src \
					  $(PACKAGE_PATH)/json/src \
					  $(PACKAGE_PATH)/lzma/src \
					  $(PACKAGE_PATH)/zlib/src \
					  $(PACKAGE_PATH)/crypto/src \
					  $(PACKAGE_PATH)/pbc/src

PACKAGE_INC_PATHS   = $(PACKAGE_PATH)/iconv/include \
					  $(PACKAGE_PATH)/lzma/include \
					  $(PACKAGE_PATH)/zlib/include \
					  $(PACKAGE_PATH)/zlib/zlib_pal/include \
					  $(PACKAGE_PATH)/crypto/include \
					  $(PACKAGE_PATH)/pbc/inlcude

#PACKAGE_SRC_FILES += $(wildcard *.c)
PACKAGE_SRC_FILES += gb2312_to_ucs2.c \
					 iconv.c \
					 ucs2_to_gb2312.c \
					 utf8_to_ucs2.c  \
					 fpconv.c \
                     lua_cjson.c \
                     strbuf.c \
					 LzmaDec.c \
                     LzmaLib.c \
					 adler32.c    \
                     compress.c   \
                     crc32.c      \
                     deflate.c    \
                     gzclose.c    \
                     gzlib.c      \
                     gzread.c     \
                     gzwrite.c    \
                     infback.c    \
                     inffast.c    \
                     inflate.c    \
                     inftrees.c   \
                     trees.c      \
                     uncompr.c    \
                     zutil.c      \
					 crypto.c     \
					 aliyun_iot_common_base64.c \
					 I_aes.c  \
					 xxtea.c	\
					 alloc.c   \
					 array.c \
					 bootstrap.c  \
					 context.c \
					 decode.c \
					 map.c  \
					 pattern.c \
					 pbc-lua.c \
					 proto.c \
					 register.c \
					 rmessage.c \
					 stringpool.c \
					 varint.c \
					 wmessage.c


# These are the tool flags specific to the iconv package only.
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

