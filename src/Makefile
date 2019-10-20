include ../aui.cfg

AUI_DIR_ROOT=$(shell cd ../;pwd)

########################## ONLY TDS USE THIS Makefile ########################## 
AUI_DIR_SRC=$(AUI_DIR_ROOT)/src/tds
AUI_DIR_INC=$(AUI_DIR_ROOT)/inc
AUI_DIR_LIB=$(AUI_DIR_ROOT)/lib/tds
AUI_DIR_OBJ_TMP=$(AUI_DIR_ROOT)/objs
AUI_DIR_OUTPUT=$(AUI_DIR_ROOT)/output/tds
AUI_DIR_OUTPUT_DDK=$(AUI_DIR_ROOT)/../ddk

AUI_SRC := aui_dis.c  aui_os.c aui_common.c \
	aui_snd.c aui_dsc.c aui_tsi.c aui_otp.c \
	aui_dog.c aui_rtc.c aui_uart.c aui_input.c aui_panel.c aui_dmx.c aui_hdmi.c \
	 aui_flash.c aui_nim.c  aui_stc.c \
	 aui_misc.c  \
	aui_trng.c aui_gpio.c aui_i2c.c aui_log.c

ifdef  _CAS9_VSC_ENABLE_
	AUI_SRC += aui_conaxvsc.c
endif

ifndef _BUILD_OTA_E_
	AUI_SRC += aui_decv.c aui_av.c aui_deca.c  aui_fs.c\
	aui_smc.c aui_tsg.c aui_image.c aui_music.c aui_mp.c \
	aui_vbi.c aui_cic.c aui_kl.c\
	aui_pvr.c cas9_pvr.c aui_ca_pvr.c
endif

ifeq ($(AUI_COMPILE_ITEM_OSD_HARDWARE),YES)
	AUI_SRC+=aui_gfx.c
else
	AUI_SRC+=aui_osd.c
endif

AUI_MACRO=

ifeq ($(AUI_COMPILE_ITEM_CARDLESS_SUPPORT),YES)
	AUI_MACRO += -DAUI_CARDLESS_SUPPORT
endif
ifeq ($(AUI_DEBUG_FLAG),YES)
	AUI_MACRO+=-D_RD_DEBUG_
endif

ifdef _BUILD_OTA_E_
AUI_MACRO+=-D_AUI_OTA_
endif

ifeq ($(AUI_COMPILE_ITEM_CARDLESS_SUPPORT),YES)
	AUI_MACRO += -DAUI_CARDLESS_SUPPORT
endif

PROJECT_OBJ := $(patsubst %.c,%.o,$(AUI_SRC) )

LIB_NAME_STATIC=libaui.a
TARGET_LIB_NAME_STATIC=$(AUI_DIR_OUTPUT)/$(LIB_NAME_STATIC)

MKDIR := @mkdir -p

TARGETS = libaui


CFLAGS +=  -Werror -Wall $(AUI_MACRO) -I$(AUI_DIR_INC) -Wunused-parameter -Wunused-function 
# Cygwin gcc not support -fno-inline-small-functions 
ifeq ($(COMPILE_ENVIRONMENT),LINUX)
CFLAGS += -fno-inline-small-functions 
endif





#PROJECT_ALL_OBJS := $(PROJECT_OBJ)


PROJECT_ALL_OBJS := $(addprefix tds/, $(PROJECT_OBJ))
#$(info "PROJECT_ALL_OBJS is ${PROJECT_ALL_OBJS}")

all: $(PROJECT_ALL_OBJS)
	@$(MKDIR) $(AUI_DIR_OUTPUT)
	@$(AR) -rc $(TARGET_LIB_NAME_STATIC) $(PROJECT_ALL_OBJS)
	@cp $(TARGET_LIB_NAME_STATIC) $(AUI_DIR_OUTPUT_DDK)

%.o : %.c
	@echo "Compiling " $@ "..."
	@$(MKDIR) $(AUI_DIR_OBJ_TMP)
	@$(CC) -c $(CFLAGS) $< -o $@



clean:
	rm -fr ./tds/*.o
	rm -fr $(AUI_DIR_OBJ_TMP)

	rm -fr $(TARGET_LIB_NAME_STATIC)
	rm -fr $(AUI_DIR_OUTPUT)


.PHONY: all static_lib
