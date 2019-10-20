
# TOPDIR always points to the AUI root directory
# special redefinitions to allow checking TDS compilation
# with Linux AUI config

ifeq ($(findstring tds_check,$(MAKECMDGOALS)),tds_check)
EXT_ALI_INC_DIR=$(_ALI_SDK_PKG_DIR_ROOT)/tds_inc
PLATFORM=TDS
COMPILER_ROOT_DIR=/opt/mips-4.3/
endif

SDK_ROOT=$(TOPDIR)/$(ALI_SDK_PKG_DIR_ROOT)

_ALI_LIB_INSTALL=$(TOPDIR)/$(ALI_LIB_INSTALL)
_ALI_SDK_PKG_DIR_ROOT=$(TOPDIR)/$(ALI_SDK_PKG_DIR_ROOT)

AUI_DIR_INC=$(TOPDIR)/inc

ifeq ($(PLATFORM),TDS)
AUI_DIR_SRC?=$(TOPDIR)/src/tds
AUI_DIR_LIB=$(TOPDIR)/lib/tds
AUI_DIR_OUTPUT=$(TOPDIR)/output/tds

EXT_ALI_INC_DIR?=$(_ALI_SDK_PKG_DIR_ROOT)/inc
EXT_ALI_INC_DIR_HLD=$(EXT_ALI_INC_DIR)/hld
EXT_ALI_INC_DIR_API=$(EXT_ALI_INC_DIR)/api
EXT_ALI_INC_DIR_BUS=$(EXT_ALI_INC_DIR)/bus
EXT_ALI_INC_DIR_LIBC=$(EXT_ALI_INC_DIR_API)/libc
endif

ifeq ($(PLATFORM),LINUX)
AUI_DIR_SRC=$(TOPDIR)/src/linux
AUI_DIR_LIB=$(TOPDIR)/lib/linux
AUI_DIR_OUTPUT=$(TOPDIR)/output/linux

LIB_DIR=$(TOPDIR)/lib/linux
SHARELIB=sharelib-src
SHARELIB_DIR=$(LIB_DIR)/$(SHARELIB)
SHARELIB_INSTALL=$(LIB_DIR)/sharelib


ifeq ($(BUILDROOT),y)
ALI_SHARE_LIB_INC=$(STAGING_DIR)/usr/include
ALI_SHARE_LIB_LIB=$(_ALI_LIB_INSTALL)/lib
ALI_DIRECTFB_HEADER=$(STAGING_DIR)/usr/include
ALI_DIRECTFB_LIB=$(STAGING_DIR)/usr/lib
ALI_DRIVER_HEADFIEL_ROOT=$(STAGING_DIR)/usr/include
else
ALI_SHARE_LIB_INC=$(SHARELIB_INSTALL)/include/aliplatform
ALI_SHARE_LIB_LIB=$(_ALI_LIB_INSTALL)/lib
ALI_DIRECTFB_HEADER=$(_ALI_SDK_PKG_DIR_ROOT)/extern/fs_include/directfb
ALI_DIRECTFB_LIB=$(_ALI_SDK_PKG_DIR_ROOT)/extern/fs_install/usr/lib
ALI_DRIVER_HEADFIEL_ROOT=$(_ALI_SDK_PKG_DIR_ROOT)/kernel/linux/include/ali_common
endif

ifeq ($(CPU_TYPE),ARM)
ifeq ($(BUILDROOT),y)
PDK_DRV_INC_DIR=$(STAGING_DIR)/usr/include
PDK_COMMON_DIR=$(STAGING_DIR)/usr/include
PDK_SDK_INC=$(STAGING_DIR)/usr/include
PDK_SDK_HAL_INC=$(STAGING_DIR)/usr/include
else
PDK_DRV_INC_DIR=$(_ALI_SDK_PKG_DIR_ROOT)/linux/kernel/alidrivers/include
PDK_COMMON_DIR=$(_ALI_SDK_PKG_DIR_ROOT)/alicommon
PDK_SDK_INC=$(_ALI_SDK_PKG_DIR_ROOT)/linux/fsmodules/alisrc/tools/sdk/inc
PDK_SDK_HAL_INC=$(_ALI_SDK_PKG_DIR_ROOT)/linux/fsmodules/alisrc/tools/sdk/inc/hal
endif
endif

endif

all:

test:
	@echo PLATFORM=$(PLATFORM)
	@echo test make
	@echo ----AUI_SRC=$(AUI_SRC)----
	@echo ----PROJECT_OBJ=$(PROJECT_OBJ)----
	@echo ----COMPILE_ENVIRONMENT=$(COMPILE_ENVIRONMENT)----
	@echo ----SEARCH_LINUX_STR_TMP=$(SEARCH_LINUX_STR_TMP)----
	@echo ----SEARCH_CYGWIN_STR_TMP=$(SEARCH_CYGWIN_STR_TMP)----
	@echo ----SEARCH_LINUX_STR_LOW=$(SEARCH_LINUX_STR_LOW)----
	@echo ----SEARCH_LINUX_STR_TOP=$(SEARCH_LINUX_STR_TOP)----
	@echo ----SEARCH_LINUX_STR_3TH=$(SEARCH_LINUX_STR_3TH)----
	@echo ----SEARCH_CYGWIN_STR_LOW=$(SEARCH_CYGWIN_STR_LOW)----
	@echo ----SEARCH_CYGWIN_STR_TOP=$(SEARCH_CYGWIN_STR_TOP)----
	@echo ----SEARCH_CYGWIN_STR_3TH=$(SEARCH_CYGWIN_STR_3TH)----
ifeq ($(PLATFORM),LINUX)
	@echo ----ABS_SHARELIB_INSTALL=$(ABS_SHARELIB_INSTALL)
	@echo ----SHARELIB_INSTALL=$(SHARELIB_INSTALL)
endif
	@echo ++++++++$(PROJECT_ALL_OBJS)++++++++++
	@echo TOPDIR=$(TOPDIR)
	@echo AUI_DIR_INC=$(AUI_DIR_INC)
	@echo AUI_DIR_LIB=$(AUI_DIR_LIB)
