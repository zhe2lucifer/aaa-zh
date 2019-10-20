TOPDIR=..

include $(TOPDIR)/aui.cfg
include $(TOPDIR)/build/build.mk

AUI_DIR_INC += -Iframework -Isample_src -I.

AUI_SRC = framework/aui_test_app.c \
	framework/aui_test_app_cmd.c \
	sample_src/aui_test_stream.c \
	sample_src/aui_test_stream_nim.c \
	sample_src/aui_test_stream_nim_dsc.c \
	sample_src/aui_test_stream_nim_dsc_kl.c \
	sample_src/aui_test_stream_tsg.c \
	sample_src/aui_test_stream_tsg_dsc.c \
	sample_src/aui_test_stream_tsg_dsc_kl.c \
	sample_src/aui_test_stream_play_live_stream.c \
	sample_src/aui_test_stream_scan_plp.c \
	sample_src/aui_av_test.c \
	sample_src/aui_deca_test.c \
	sample_src/aui_script_test.c \
	sample_src/aui_dis_test.c \
	sample_src/aui_flash_test.c \
	sample_src/aui_kl_tests.c \
	sample_src/aui_smc_test.c \
	sample_src/aui_snd_test.c \
	sample_src/aui_gpio_test.c \
	sample_src/aui_vbi_test.c \
	sample_src/aui_dmx_test.c \
	sample_src/aui_decv_test.c \
	sample_src/aui_hdmi_test.c \
	sample_src/aui_nim_init.c \
	sample_src/aui_osd_test.c \
	sample_src/aui_panel_test.c \
	sample_src/aui_dsc_test.c \
	sample_src/aui_help_print.c \
	sample_src/aui_uart_test.c \
	sample_src/aui_sys_setting.c

# The ($(PLATFORM),TDS) case is for building alidownloader(aui_demo) in buildroot environment
ifeq ($(PLATFORM),TDS)
AUI_SRC += tds_aui_test_main.c \
	framework/unity.c \
	framework/unity_fixture.c \
	sample_src/aui_dog_test.c \
	sample_src/aui_music_test.c \
	sample_src/aui_os_test.c \
	sample_src/aui_pvr_test.c \
	sample_src/aui_rtc_test.c \
	sample_src/aui_image_test.c \
	sample_src/aui_fs_test.c \
	sample_src/aui_nim_test.c \
	sample_src/aui_nim_tests.c \
	sample_src/aui_input_test.c \
	sample_src/aui_channel_change_test.c \
	sample_src/aui_trng_test.c  \
	sample_src/aui_misc_test.c  \
	sample_src/aui_cic_test.c  \
	sample_src/aui_i2c_test.c  \
	sample_src/aui_ca_test.c	\
	sample_src/aui_mp_test.c    \
	sample_src/aui_test_stream_decrypt.c
#ifneq ($(AUI_COMPILE_ITEM_SUBT_SUPPORT), NO)
#AUI_SRC += sample_src/aui_ttx_test.c sample_src/aui_subtitle_test.c	
#AUI_MACRO += -DAUI_SUBT_SUPPORT
#endif	

AUI_MACRO += -DTEST_APP_CMD  

endif

ifeq ($(PLATFORM),LINUX)
AUI_SRC += linux_aui_test_main.c \
	sample_src/aui_nim_tests.c \
	sample_src/aui_av_injecter_test.c \
	sample_src/aui_av_injecter_audio_mix_test.c \
	sample_src/aui_misc_test.c \
	sample_src/aui_dog_test.c \
	sample_src/aui_rtc_test.c \
	sample_src/aui_ini_config.c	\
	sample_src/aui_channel_change_test.c \
	sample_src/aui_multi_nim_test.c \
	sample_src/aui_trng_test.c \
	sample_src/aui_cic_tests.c  \
	sample_src/aui_i2c_test.c   \
	sample_src/aui_test_stream_cic_tsg.c \
	sample_src/aui_pip_test.c \
	sample_src/aui_pip_test_live.c \
	sample_src/aui_pip_test_media.c \
	sample_src/aui_dmx_record_test.c
	
ifeq ($(BUILDROOT_BUILD_AUI_VMX_PLUS),y)
AUI_SRC += sample_src/aui_vmx_dsc_test.c
AUI_MACRO += -DVMX_PLUS_SUPPORT
endif	

ifeq ($(AUI_COMPILE_ITEM_VSC_SUPPORT), YES)
AUI_SRC +=	sample_src/aui_test_conaxvsc.c
AUI_MACRO += -DLINUX_VSC_SUPPORT

ifeq ($(BUILDROOT_BUILD_AUI_VSC_SMI), y)
AUI_MACRO += -DVSC_SMI
endif

ifeq ($(BUILDROOT_BUILD_AUI_CF_SMI), y)
AUI_MACRO += -D_SMI_CF_ENABLE_
endif

endif
	
ifeq ($(AUI_COMPILE_ITEM_PVR_SUPPORT),YES)
AUI_SRC += sample_src/aui_pvr_test.c
AUI_SRC += sample_src/aui_pvr_reencrypt_test.c
AUI_SRC += sample_src/encrypt_pvr.c
AUI_SRC += sample_src/aui_qt_pvr_board_test.c
AUI_MACRO += -DLINUX_PVR_SUPPORT
endif
ifeq ($(AUI_COMPILE_ITEM_MP_SUPPORT),YES)
AUI_SRC += sample_src/aui_mp_test.c
AUI_MACRO += -DLINUX_MP_SUPPORT
endif
ifeq ($(AUI_PACKAGE_ALIPLATFORM_INPUT),y)
AUI_SRC += sample_src/aui_input_test.c
CFLAGS += -DAUI_PACKAGE_ALIPLATFORM_INPUT
endif

ifeq ($(BUILD_TEE_AUI),y)
AUI_MACRO += -DAUI_TA_SUPPORT
AUI_SRC += sample_src/aui_ta_client_test.c
endif

AUI_MACRO += -DTEST_APP_CMD
endif

PROJECT_OBJ := $(patsubst %.c,%.o,$(AUI_SRC) )

#the AUI debug print switch flag, set "YES" when debug mode.
ifeq ($(AUI_DEBUG_FLAG),YES)
AUI_MACRO+=-D_RD_DEBUG_
endif

#Set this item equal to "YES" when project support media player, otherwise set "NO"
ifeq ($(AUI_COMPILE_ITEM_MP_SUPPORT),YES)
AUI_MACRO+=-DSUPPORT_MPEG4_TEST
endif

#Set this item equal to "YES" if IC is dual cpu, otherwise set "NO"
ifeq ($(AUI_COMPILE_ITEM_DUAL_CPU),YES)
AUI_MACRO+=-DDUAL_ENABLE
endif

#Set this item equal to "YES" when project open AS feature, otherwise set "NO"
ifeq ($(AUI_COMPILE_ITEM_AS_SUPPORT),YES)
AUI_MACRO+=-DCAS9_PVR_SUPPORT
endif

LIB_NAME_STATIC=libaui_test.a
TARGET_LIB_NAME_STATIC=$(AUI_DIR_OUTPUT)/$(LIB_NAME_STATIC)

#TARGET = aui_test
TARGETS_STATIC = aui_test
TARGETS_DYNAMIC = aui_test_dynamic

ifeq ($(PLATFORM),TDS)
CFLAGS += -DAUI $(AUI_MACRO) \
	-I$(AUI_DIR_INC) \
	-I$(EXT_ALI_INC_DIR) \
	-I$(EXT_ALI_INC_DIR_HLD) \
	-I$(EXT_ALI_INC_DIR_LIBC) \
	-I$(EXT_ALI_INC_DIR_BUS) \
	-I$(EXT_ALI_INC_DIR_API)

CFLAGS += -fno-builtin-printf -Wno-unused-function \
	-Wno-format -Wno-implicit-function-declaration \
	-Wno-parentheses -Wno-return-type -Wno-unused-variable \
	-Wno-unused-parameter -Wno-implicit -Wno-pointer-sign

#LDFLAG = -L$(AUI_DIR_LIB)

endif

ifeq ($(PLATFORM),LINUX)

CFLAGS += -DAUI $(AUI_MACRO) \
	-I$(AUI_DIR_INC) \
	-I$(ALI_SHARE_LIB_INC) \
	-I$(ALI_SHARE_LIB_INC)/curl \
	-I$(ALI_DRIVER_HEADFIEL_ROOT)

ifeq ($(AUI_COMPILE_ITEM_PVR_SUPPORT),YES)
CFLAGS += -I$(STAGING_DIR)/usr/include \
	-I$(STAGING_DIR)/usr/include/alihld-sl/inc
endif

ifeq ($(BUILDROOT),y)
CFLAGS += -I$(AUI_DIR_INC) \
	-I$(STAGING_DIR)/usr/include/ali_common \
	-I$(STAGING_DIR)/usr/include/aliplatform
endif

CFLAGS += -Wno-unused-function \
	-Wno-format \
	-Wno-unused-variable \
	-Wno-unused-parameter \
	-Wno-pointer-sign
	
CFLAGS += -Wall -Werror

#LDFLAG = -L$(ALI_SHARE_LIB_LIB) -L$(TOPDIR)/output/linux \
#	-laui -laliplatform -lalislce -lalislcic -lalislcli \
#	-lalisldis -lalisldmx -lalisldsc -lalislnim \
#	-lalislsdec -lalislsmc -lalislsnd -lalislstorage \
#	-lalisltsg -lalisltsi \
#	-lalislvbi -lalislvdec -lalislwatchdog -laliunity -lalislcli \
#	-lalislsmc -lalislhdmi -lalislstandby -ldl -lalislotp \
#	-ldirect -ldirectfb -lfusion -lalislsbm -lalislavsync

LDFLAG_STATIC = -L$(ALI_SHARE_LIB_LIB) -L$(TOPDIR)/output/linux \
	-L$(STAGING_DIR)/usr/lib  \
	-laui_static -ldl -lrt -lm

ifeq ($(BUILD_TEE_AUI),y)
LDFLAG_STATIC += -lotzapi
endif

ifeq ($(AUI_COMPILE_ITEM_MP_SUPPORT),YES)
	LDFLAG_STATIC += -lnmpgoplayer
endif

ifneq ($(BUILDROOT_DISABLE_AUI_GFX_API),y)
LDFLAG_STATIC += -ldirect -ldirectfb -lfusion
endif

LDFLAG_DYNAMIC = -L$(TOPDIR)/output/linux \
	-L$(ALI_SHARE_LIB_LIB) -laui 

ifeq ($(BUILD_TEE_AUI),y)
LDFLAG_DYNAMIC+=-L$(STAGING_DIR)/usr/lib -lotzapi
endif

ifeq ($(AUI_COMPILE_ITEM_PVR_SUPPORT),YES)
#LDFLAG += -L$(STAGING_DIR)/usr/lib/alipvr-lib/ \
#	-L$(TARGET_DIR)/usr/lib/ \
#	-lalisltrng \
#	-lpthread -ldl -lrt -lc -lm -lhld-sl


# note: Static library link is a fixed order according to the relations between the invocation of the static library
LDFLAG_STATIC += -L$(STAGING_DIR)/usr/lib/ \
	-Wl,-Bstatic \
	-lspvr \
	-lpvr_eng \
	-lalichunk \
	-lalic \
	-lalifs \
	-lalirsa \
	-lbase \
	-lhld-sl \
	-Wl,-Bdynamic	

LDFLAG_DYNAMIC += -L$(STAGING_DIR)/usr/lib/ \
	-L$(TARGET_DIR)/usr/lib/ \
	-lpthread -ldl -lrt -lc -lm -lspvr -lalichunk -lalic -lalifs -lalirsa -lbase -lhld-sl
endif

ifneq ($(BUILDROOT),y)
LDFLAG += -lalisltrng
endif

ifeq ($(CPU_TYPE),MIPS)
LDFLAGS += -lalislupgradeinfo -lalislupgrade -lalislupgradeusb
LDFLAG_STATIC += -lpthread
LDFLAG_DYNAMIC += -lpthread
endif

ifeq ($(CPU_TYPE),ARM)
CFLAGS += -I$(PDK_DRV_INC_DIR) \
	-I$(PDK_COMMON_DIR) \
	-I$(PDK_SDK_INC) \
	-I$(PDK_SDK_HAL_INC)

ifeq ($(BUILD_TEE_AUI),y)
CFLAGS+=-I$(STAGING_DIR)/usr/include/ali_common/alitee/include
endif

#LDFLAG += -lpthread
LDFLAG_STATIC += -lpthread
LDFLAG_DYNAMIC += -lpthread
endif

ifeq ($(BUILDROOT_BUILD_AUI_CF_SMI), y)
LDFLAG_STATIC += -lcfsmk
LDFLAG_DYNAMIC += -lcfsmk
endif

ifeq ($(AUI_COMPILE_ITEM_GFX_SUPPORT), YES)
ifneq ($(BUILDROOT_DISABLE_AUI_GFX_API),y)
LDFLAG_STATIC += -L$(ALI_DIRECTFB_LIB) -ldirectfb -lfusion -ldirect -lz
LDFLAG_DYNAMIC += -L$(ALI_DIRECTFB_LIB) -ldirectfb -lfusion -ldirect -lz
endif
endif

endif # LINUX

PROJECT_ALL_OBJS := $(PROJECT_OBJ)

CFLAGS += $(APP_CFLAGS)

all: $(PROJECT_ALL_OBJS) app

ifeq ($(PLATFORM),TDS)
app:
	$(MKDIR) $(AUI_DIR_OUTPUT)
	@$(AR) -rc $(TARGET_LIB_NAME_STATIC) $(PROJECT_ALL_OBJS)

install: all

%.o : %.c
	@echo Compiling %@ ...
	@$(CC) -c $(CFLAGS) $< -o $@

endif # TDS

ifeq ($(PLATFORM),LINUX)
install: all
	cp $(AUI_DIR_OUTPUT)/$(TARGETS_STATIC) $(TOPDIR)/$(ALI_APP_INSTALL)

# Application link
app: $(PROJECT_ALL_OBJS)
#	$(CC) $(PROJECT_ALL_OBJS) -o $(AUI_DIR_OUTPUT)/$(TARGET) $(LDFLAG)
#	@$(CC) $(PROJECT_ALL_OBJS) -o $(AUI_DIR_OUTPUT)/$(TARGETS_STATIC) $(LDFLAG_STATIC)
	@$(CC) $(PROJECT_ALL_OBJS) -o $(AUI_DIR_OUTPUT)/$(TARGETS_DYNAMIC) $(LDFLAG_DYNAMIC)

lib: $(PROJECT_ALL_OBJS)
	$(MKDIR) $(AUI_DIR_OUTPUT)
	@$(AR) -rc $(TARGET_LIB_NAME_STATIC) $(PROJECT_ALL_OBJS)

DEPS := $(PROJECT_ALL_OBJS:.o=.d)

-include $(DEPS)

%.o : %.c
	@echo Compiling $@ ...
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -MM -MF dep $<
	@sed -e 's|.*:|$*.o:|' < dep > $*.d

endif # LINUX

ifeq ($(BUILDROOT_BUILD_AUI_LNB_BAND),KU)
LNB_CFG=KU_BAND
endif
ifeq ($(BUILDROOT_BUILD_AUI_LNB_BAND),C)
LNB_CFG=C_BAND
endif

#Application CFLAGS
APP_CFLAGS := -DBOARD_CFG_$(BOARD_CFG) -DLNB_CFG_$(LNB_CFG) -DAUI_BOARD_VERSION_$(AUI_BOARD_VERSION)

ifeq ($(PLATFORM),LINUX)
ifeq ($(BOARD_CFG),M3627)
CFLAGS += -DSUPPORT_TWO_TUNER
endif
endif

clean:
	rm -f *.d *.o
	cd framework; rm -f *.d *.o
	cd sample_src; rm -f *.d *.o
	rm -rf $(AUI_DIR_OUTPUT)/$(TARGETS_STATIC)
	rm -fr $(TARGET_LIB_NAME_STATIC)


.PHONY = all install clean lib

MKDIR = mkdir -p
