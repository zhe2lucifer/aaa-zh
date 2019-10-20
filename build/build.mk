
include $(TOPDIR)/build/config.mk
include $(TOPDIR)/build/rules.mk

AR_PATH=
OS := $(shell uname -s)

#COMPILE_ENVIRONMENT=$(shell echo $(OS) | grep -o -i -E '(linux|cygwin)' | tr '[a-z]' '[A-Z]')
SEARCH_LINUX_STR_TMP  := $(shell echo $(OS) | grep -i linux)
SEARCH_CYGWIN_STR_TMP := $(shell echo $(OS) | grep -i cygwin)
SEARCH_LINUX_STR_LOW:=$(findstring linux,$(SEARCH_LINUX_STR_TMP))
SEARCH_LINUX_STR_TOP:=$(findstring LINUX,$(SEARCH_LINUX_STR_TMP))
SEARCH_LINUX_STR_3TH:=$(findstring Linux,$(SEARCH_LINUX_STR_TMP))
SEARCH_CYGWIN_STR_LOW:=$(findstring cygwin,$(SEARCH_CYGWIN_STR_TMP))
SEARCH_CYGWIN_STR_TOP:=$(findstring CYGWIN,$(SEARCH_CYGWIN_STR_TMP))
SEARCH_CYGWIN_STR_3TH:=$(findstring Cygwin,$(SEARCH_CYGWIN_STR_TMP))

ifeq ($(SEARCH_LINUX_STR_LOW),linux)
COMPILE_ENVIRONMENT=LINUX
else
	ifeq ($(SEARCH_LINUX_STR_TOP),LINUX)
	COMPILE_ENVIRONMENT=LINUX
	else
		ifeq ($(SEARCH_LINUX_STR_3TH),Linux)
		COMPILE_ENVIRONMENT=LINUX
		else
			ifeq ($(SEARCH_CYGWIN_STR_LOW),cygwin)
			COMPILE_ENVIRONMENT=CYGWIN
			else
				ifeq ($(SEARCH_CYGWIN_STR_TOP),CYGWIN)
				COMPILE_ENVIRONMENT=CYGWIN
				else
					ifeq ($(SEARCH_CYGWIN_STR_3TH),Cygwin)
					COMPILE_ENVIRONMENT=CYGWIN
					else
					COMPILE_ENVIRONMENT=
					endif
				endif
			endif
		endif
	endif
endif

ifneq ($(BUILDROOT),y)
ifeq ($(COMPILE_ENVIRONMENT),CYGWIN)
	CC := $(COMPILER_ROOT_DIR)sde-gcc
	AR=$(COMPILER_ROOT_DIR)sde-ar
endif
ifeq ($(COMPILE_ENVIRONMENT),LINUX)
	ifeq ($(PLATFORM), LINUX)
		ifeq ($(CPU_TYPE), MIPS)
			CC:=$(COMPILER_ROOT_DIR)bin/mipsel-linux-gnu-gcc
			AR:=$(COMPILER_ROOT_DIR)bin/mipsel-linux-gnu-ar
			NM:=$(COMPILER_ROOT_DIR)bin/mipsel-linux-gnu-nm
			OBJCPY:=$(COMPILER_ROOT_DIR)bin/mipsel-linux-gnu-objcopy
			HOST=mipsel-linux
		else
			CC:=$(COMPILER_ROOT_DIR)bin/arm-linux-gnueabi-gcc
			AR:=$(COMPILER_ROOT_DIR)bin/arm-linux-gnueabi-ar
			NM:=$(COMPILER_ROOT_DIR)bin/arm-linux-gnueabi-nm
			OBJCPY:=$(COMPILER_ROOT_DIR)bin/arm-linux-gnueabi-objcopy
			HOST=arm-linux
		endif
	else
		CC:=$(COMPILER_ROOT_DIR)bin/mips-sde-elf-gcc
		AR:=$(COMPILER_ROOT_DIR)bin/mips-sde-elf-ar
		NM:=$(COMPILER_ROOT_DIR)bin/mips-sde-elf-nm
		OBJCPY:=$(COMPILER_ROOT_DIR)bin/mips-sde-elf-objcopy
	endif
endif
endif

# CFLAGS
ifeq ($(PLATFORM),LINUX)
CFLAGS = -g -O1 -fPIC -fsigned-char -W -Wall -DAUI_LINUX
CFLAGS += $(BR_CFLAGS)

ifeq ($(BUILDROOT),y)
else
CFLAGS += -EL
endif

ifeq ($(CPU_TYPE),MIPS)
CFLAGS += -mips32
endif

ifeq ($(BUILDROOT_BUILD_AUI_NESTOR),y)
CFLAGS += -fno-inline
else
CFLAGS += -fno-inline-small-functions
endif

endif #endif ($(PLATFORM),LINUX)


ifeq ($(PLATFORM),TDS)
#CFLAGS = -g -O1 -fsigned-char -W -Wall
CFLAGS = -O1 -fsigned-char -W -Wall

ifeq ($(CPU_TYPE),MIPS)
CFLAGS += -EL -mips32
CFLAGS += -msoft-float  -fno-inline-small-functions 
endif

CFLAGS += -DAUI_TDS -DAUI

AUI_DIR_ROOT=$(shell cd ../;pwd)
EXT_ALI_INC_DIR=$(AUI_DIR_ROOT)/../inc
EXT_ALI_INC_DIR_HLD=$(EXT_ALI_INC_DIR)/hld
EXT_ALI_INC_DIR_API=$(EXT_ALI_INC_DIR)/api
EXT_ALI_INC_DIR_BUS=$(EXT_ALI_INC_DIR)/bus
EXT_ALI_INC_DIR_LIBC=$(EXT_ALI_INC_DIR)/api/libc
CFLAGS += -I$(AUI_DIR_INC) \
					-I$(EXT_ALI_INC_DIR) \
					-I$(EXT_ALI_INC_DIR_HLD) \
					-I$(EXT_ALI_INC_DIR_LIBC) \
					-I$(EXT_ALI_INC_DIR_BUS)
endif #endif ($(PLATFORM),TDS)


#$(info "Compiler environment is: $(COMPILE_ENVIRONMENT)")
#$(info "Platform is: $(PLATFORM)")
