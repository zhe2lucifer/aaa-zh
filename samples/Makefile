include ../aui.cfg

AUI_DIR_ROOT=$(shell cd ../;pwd)
#*************************Do not modify the config below*****************************************

AUI_DIR_SRC=$(AUI_DIR_ROOT)/src/tds
AUI_DIR_INC=$(AUI_DIR_ROOT)/inc
AUI_DIR_TEST_INC=$(AUI_DIR_ROOT)/samples
AUI_DIR_TEST_MODULE_INC=$(AUI_DIR_ROOT)/samples/sample_src
AUI_DIR_TEST_UNITY_INC=$(AUI_DIR_ROOT)/samples/framework
AUI_DIR_OBJ_TMP=$(AUI_DIR_ROOT)/objs
AUI_DIR_OUTPUT=$(AUI_DIR_ROOT)/output/tds
AUI_DIR_OUTPUT_DDK=$(AUI_DIR_ROOT)/../ddk

#the same part
AUI_SRC = tds_aui_test_main.c					\
			framework/aui_test_app.c			\
			framework/aui_test_app_cmd.c		\
			sample_src/aui_help_print.c			\
			sample_src/aui_script_test.c		\
			sample_src/aui_flash_test.c			\
			sample_src/aui_sys_setting.c
			
ifndef _AUI_TEST_MINI_SUPPORT_
AUI_SRC +=	sample_src/aui_test_stream.c		\
			sample_src/aui_test_stream_nim.c 	\
			sample_src/aui_test_stream_nim_dsc.c 	\
			sample_src/aui_test_stream_nim_dsc_kl.c \
			sample_src/aui_test_stream_tsg.c 		\
			sample_src/aui_test_stream_tsg_dsc.c 	\
			sample_src/aui_test_stream_tsg_dsc_kl.c \
			sample_src/aui_test_stream_play_live_stream.c \
			sample_src/aui_test_stream_scan_plp.c \
			sample_src/aui_av_test.c			\
			sample_src/aui_deca_test.c			\
			sample_src/aui_dis_test.c			\
			sample_src/aui_kl_tests.c			\
			sample_src/aui_mp_test.c			\
			sample_src/aui_smc_test.c			\
			sample_src/aui_snd_test.c			\
			sample_src/aui_vbi_test.c			\
			sample_src/aui_dmx_test.c			\
			sample_src/aui_decv_test.c			\
			sample_src/aui_hdmi_test.c			\
			sample_src/aui_nim_init.c			\
			sample_src/aui_dsc_test.c			\
			sample_src/aui_uart_test.c			\
			sample_src/aui_trng_test.c			\
			sample_src/aui_dog_test.c 			\
		  	sample_src/aui_music_test.c			\
		  	sample_src/aui_os_test.c 			\
			sample_src/aui_pvr_test.c 			\
			sample_src/aui_pvr_reencrypt_test.c \
			sample_src/encrypt_pvr.c	\
			sample_src/aui_qt_pvr_board_test.c 	\
			sample_src/aui_rtc_test.c			\
			sample_src/aui_image_test.c			\
			sample_src/aui_fs_test.c			\
			sample_src/aui_osd_test.c			\
			sample_src/aui_nim_test.c			\
			sample_src/aui_nim_tests.c			\
			sample_src/aui_panel_test.c 		\
			sample_src/aui_input_test.c 		\
			sample_src/aui_misc_test.c 			\
			sample_src/aui_cic_test.c			\
			sample_src/aui_i2c_test.c			\
			sample_src/aui_channel_change_test.c \
			sample_src/aui_ca_test.c			\
			sample_src/aui_test_stream_decrypt.c \
			sample_src/aui_dmx_record_test.c
endif

ifdef _CAS9_VSC_ENABLE_
AUI_SRC +=	sample_src/aui_test_conaxvsc.c
endif

AUI_MACRO+=-DAUI_TDS -DTEST_APP_CMD



PROJECT_OBJ := $(patsubst %.c,%.o,$(AUI_SRC) )

#the AUI debug print switch flag, set "YES" when debug mode.
ifeq ($(AUI_DEBUG_FLAG),YES)
AUI_MACRO+=-D_RD_DEBUG_
endif

ifdef _BUILD_OTA_E_
AUI_MACRO+=-D_AUI_OTA_
endif

#Set this item equal to "YES" when project open AS feature, otherwise set "NO"
ifeq ($(AUI_COMPILE_ITEM_AS_SUPPORT),YES)
AUI_MACRO+=-DCAS9_PVR_SUPPORT
endif

# Board configuration : M3733 or M3515
AUI_MACRO+=-DBOARD_CFG_$(BOARD_CFG)
AUI_MACRO+=-DAUI_BOARD_VERSION_$(AUI_BOARD_VERSION)

LIB_NAME_STATIC=libaui_test.a
TARGET_LIB_NAME_STATIC=$(AUI_DIR_OUTPUT)/$(LIB_NAME_STATIC)

MKDIR := mkdir -p

TARGETS = aui_test

CFLAGS += -fno-builtin-printf \
					 $(AUI_MACRO) \
					-I$(AUI_DIR_INC) \
					-I$(AUI_DIR_TEST_INC)\
					-I$(AUI_DIR_TEST_MODULE_INC)\
					-I$(AUI_DIR_TEST_UNITY_INC)\
					-I$(EXT_ALI_INC_DIR) \
					-I$(EXT_ALI_INC_DIR_HLD) \
					-I$(EXT_ALI_INC_DIR_LIBC) \
					-I$(EXT_ALI_INC_DIR_BUS) \
					-I$(EXT_ALI_INC_DIR_API)
LDFLAG := -L$(AUI_DIR_LIB)

CFLAGS += -Wall -Werror

# Cygwin gcc not support these option
ifeq ($(COMPILE_ENVIRONMENT),LINUX)
CFLAGS += -fno-inline-small-functions -Wno-pointer-sign
endif

ifeq ($(PLATFORM),TDS)
PROJECT_ALL_OBJS := $(PROJECT_OBJ)
endif

all: $(PROJECT_ALL_OBJS)
	@$(MKDIR) $(AUI_DIR_OUTPUT)
	@$(AR) -rc $(TARGET_LIB_NAME_STATIC) $(PROJECT_ALL_OBJS)
	cp $(TARGET_LIB_NAME_STATIC) $(AUI_DIR_OUTPUT_DDK)

%.o : %.c
	@echo "Compiling " $@ "..."
	@$(MKDIR) $(AUI_DIR_OBJ_TMP)
	@$(CC) -c $(CFLAGS) $< -o $@


clean:
	rm -fr ./*.o .depend
	rm -fr ./sample_src/*.o 
	rm -fr ./framework/*.o
	rm -fr $(AUI_DIR_OBJ_TMP)
	rm -fr $(TARGETS)
	rm -fr $(AUI_DIR_OUTPUT)
	#rm -fr $(TARGET_LIB_NAME_STATIC)

.PHONY = bin all static_lib install
