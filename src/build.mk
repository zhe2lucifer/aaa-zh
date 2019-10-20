TOPDIR=..

include $(TOPDIR)/aui.cfg
include $(TOPDIR)/build/build.mk

AUI_SRC = aui_common.c          \
	aui_dis.c               \
	aui_decv.c              \
	aui_av.c                \
	aui_deca.c              \
	aui_snd.c               \
	aui_dsc.c               \
	aui_tsi.c               \
	aui_otp.c               \
	aui_gfx.c               \
	aui_kl.c                \
	aui_dog.c               \
	aui_uart.c              \
	aui_dmx.c               \
	aui_hdmi.c              \
	aui_nim.c               \
	aui_smc.c               \
	aui_vbi.c               \
	aui_tsg.c               \
	aui_music.c             \
	aui_flash.c             \
	aui_panel.c             \
	aui_trng.c              \
	aui_misc.c              \
	aui_rtc.c               \
	aui_stc.c               \
	aui_cic.c               \
	aui_i2c.c               \
	aui_log.c

ifeq ($(PLATFORM),TDS)
AUI_SRC += aui_os.c             \
	aui_fs.c                \
	aui_image.c             \
	aui_pvr.c               \
	aui_input.c             \
	aui_mp.c                \
	aui_gpio.c
	
ifeq ($(AUI_COMPILE_ITEM_AS_SUPPORT),YES)
AUI_SRC += cas9_pvr.c
endif
ifeq ($(AUI_COMPILE_ITEM_PVR_SUPPORT),YES)
AUI_SRC += aui_pvr.c cas9_pvr.c aui_ca_pvr.c
endif

AUI_MACRO=

#ifneq ($(AUI_COMPILE_ITEM_SUBT_SUPPORT),NO)
#AUI_SRC+=aui_ttx.c aui_subtitle.c
#AUI_MACRO += -DAUI_SUBT_SUPPORT
#endif

#Set this item equal to "YES" if IC support hardware GE, otherwise set "NO"
ifeq ($(AUI_COMPILE_ITEM_OSD_HARDWARE),YES)
AUI_SRC += aui_gfx.c
else
AUI_SRC += aui_osd.c
endif

endif # PLATFORM TDS

ifeq ($(PLATFORM),LINUX)
AUI_SRC += \
	aui_av_injecter.c \
	aui_gpio.c

ifeq ($(AUI_COMPILE_ITEM_PVR_SUPPORT),YES)
AUI_SRC += aui_pvr.c aui_ca_pvr.c
AUI_SRC += c200a_pvr.c
AUI_SRC += gen_ca_pvr.c
ifeq ($(BUILDROOT_BUILD_AUI_PVR_VSC_PVR),y)
AUI_SRC += cas9_cl_pvr.c
else
AUI_SRC += cas9_pvr.c
endif
endif


ifeq ($(AUI_COMPILE_ITEM_VSC_SUPPORT),YES)
AUI_SRC += aui_conaxvsc.c
endif

ifeq ($(AUI_COMPILE_ITEM_MP_SUPPORT),YES)
AUI_SRC += aui_mp.c
endif

ifeq ($(AUI_PACKAGE_ALIPLATFORM_INPUT),y)
AUI_SRC += aui_input.c
endif

ifeq ($(BUILDROOT_BUILD_AUI_VMX_PLUS),y)
AUI_SRC += aui_vmx.c
endif

AUI_MACRO=

#Set this item equal to "YES" if IC support hardware GE, otherwise set "NO"
ifeq ($(AUI_COMPILE_ITEM_OSD_HARDWARE),YES)
AUI_SRC +=
else
AUI_SRC +=
endif

ALIPLATFORM_STATIC_LIB :
	mkdir -p $(TOPDIR)/output/linux/temp
ifeq ($(AUI_COMPILE_ITEM_PVR_SUPPORT),YES)
	cp $(STAGING_DIR)/usr/lib/libhld-sl.a $(TOPDIR)/output/linux/temp;
endif
	cp $(STAGING_DIR)/usr/lib/libalisl*.a $(STAGING_DIR)/usr/lib/libaliplatform.a   $(TOPDIR)/output/linux/temp/ && \
	cd $(TOPDIR)/output/linux/temp/ && \
	for file in `ls ./lib*.a` ; \
	do \
		$(AR) -xo $$file; \
	done; \
	cd -

ALIPLATFORM_DYNAMIC_LIB :
	mkdir -p $(TOPDIR)/output/linux/temp2
ifeq ($(AUI_COMPILE_ITEM_PVR_SUPPORT),YES)
	cp $(STAGING_DIR)/usr/lib/libhld-sl.a $(TOPDIR)/output/linux/temp2;
endif
	cp $(STAGING_DIR)/usr/lib/libalisl*.a $(STAGING_DIR)/usr/lib/libaliplatform.a  $(TOPDIR)/output/linux/temp2/ && \
	cd $(TOPDIR)/output/linux/temp2/ && \
	rm -fr lib*upgrade*.a && \
	for file in `ls ./lib*.a` ; \
	do \
		$(AR) -xo $$file; \
	done; \
	cd -

endif # PLATFROM LINUX


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

ifeq ($(BUILDROOT_DISABLE_AUI_GFX_API),y)
AUI_MACRO+=-DLINUX_AUI_DISABLE_GFX
endif

ifeq ($(BUILD_TEE_AUI),y)
AUI_MACRO += -DAUI_TA_SUPPORT
endif

# Board configuration : M3733 or M3515
AUI_MACRO+=-DBOARD_CFG_$(BOARD_CFG)
AUI_MACRO+=-DBOARD_VERSION_$(BOARD_VERSION)

LIB_NAME_STATIC=libaui.a
TARGET_LIB_NAME_STATIC=$(AUI_DIR_OUTPUT)/$(LIB_NAME_STATIC)

LIB_NAME_STATIC_SINGLE=libaui_static.a
TARGET_LIB_NAME_STATIC_SINGLE=$(AUI_DIR_OUTPUT)/$(LIB_NAME_STATIC_SINGLE)

LIB_NAME_DYNAMIC=libaui.so
TARGET_LIB_NAME_DYNAMIC=$(AUI_DIR_OUTPUT)/$(LIB_NAME_DYNAMIC)

LIB_NAME_DYNAMIC_SINGLE=libaui_dynamic.so
TARGET_LIB_NAME_DYNAMIC_SINGLE=$(AUI_DIR_OUTPUT)/$(LIB_NAME_DYNAMIC_SINGLE)

MKDIR := @mkdir -p


ifeq ($(PLATFORM),TDS)
CFLAGS += -DAUI_TDS -DAUI $(AUI_MACRO) \
	-I$(AUI_DIR_INC) \
	-I$(EXT_ALI_INC_DIR) \
	-I$(EXT_ALI_INC_DIR_HLD) \
	-I$(EXT_ALI_INC_DIR_LIBC) \
	-I$(EXT_ALI_INC_DIR_BUS)
endif

ifeq ($(PLATFORM),LINUX)
CFLAGS += -DAUI_LINUX -DAUI -D_GNU_SOURCE $(AUI_MACRO) \
	-fstack-protector -fstack-protector-all -Wstack-protector \
	-Werror -Wall -Wformat=2 \
	-I$(AUI_DIR_INC) \
	-I$(TOPDIR) \
	-I$(ALI_SHARE_LIB_INC) \
	-I$(ALI_SHARE_LIB_INC)/curl \
	-I$(ALI_DRIVER_HEADFIEL_ROOT) \
	-I$(ALI_DRIVER_HEADFIEL_ROOT)/../../../../platform/inc \
	-I$(ALI_DRIVER_HEADFIEL_ROOT)/../../../../platform/inc/hal

#	-I$(ALI_DIRECTFB_HEADER)

ifeq ($(AUI_COMPILE_ITEM_PVR_SUPPORT),YES)
CFLAGS += -L$(STAGING_DIR)/usr/lib/ \
	-L$(TARGET_DIR)/usr/lib/ \
	-lalisltrng \
	-lpthread -ldl -lrt -lc -lalichunk -lalic -lalifs -lalirsa -lbase -lspvr  -lm -lhld-sl \
	-I$(STAGING_DIR)/usr/include/alihld-sl/inc

endif

ifeq ($(CPU_TYPE),ARM)
CFLAGS += -I$(PDK_DRV_INC_DIR) \
	-I$(PDK_COMMON_DIR) \
	-I$(PDK_SDK_INC) \
	-I$(PDK_SDK_HAL_INC)
endif

ifeq ($(BUILDROOT),y)
CFLAGS += -I$(STAGING_DIR)/usr/include \
	-I$(STAGING_DIR)/usr/include/ali_common \
	-I$(AUI_DIR_INC) \
	-I$(STAGING_DIR)/usr/include/aliplatform
ifneq ($(BUILDROOT_DISABLE_AUI_GFX_API),y)
CFLAGS += -I$(STAGING_DIR)/usr/include/directfb
endif
endif

ifeq ($(BUILDROOT_BUILD_AUI_PVR_VSC_PVR)$(BUILDROOT_BUILD_AUI_VSC_SMI),yy)
CFLAGS += -DVSC_SMI
endif

endif


ifeq ($(PLATFORM),TDS)

PROJECT_ALL_OBJS := $(addprefix tds/, $(PROJECT_OBJ))

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean_obj:
	cd tds; rm -f *.d *.o
endif

ifeq ($(PLATFORM),LINUX)

PROJECT_ALL_OBJS := $(addprefix linux/, $(PROJECT_OBJ))
LIBFLAGS = -s -Wall -Werror -shared -Wl
LDFLAG = -L$(ALI_SHARE_LIB_LIB) -L$(TOPDIR)/output/linux \
	-L$(STAGING_DIR)/lib \
	-L$(STAGING_DIR)/usr/lib
	
LDLIB_DYNAMIC = -laliplatform \
	-lalislce \
	-lalisli2c \
	-lalislcic \
	-lalisldis \
	-lalisldmx \
	-lalisldsc \
	-lalislnim \
	-lalislsdec \
	-lalislsmc \
	-lalislsnd \
	-lalislstorage \
	-lalisltsg \
	-lalisltsi \
	-lalislvbi \
	-lalislvdec \
	-lalislwatchdog \
	-lalislhdmi \
	-lalislstandby \
	-lalislotp \
	-lalislsbm \
	-lalislavsync \
	-lalisltrng \
	-lalislevent \
	-ldl \
	-lrt \
	-lm	

ifeq ($(AUI_COMPILE_ITEM_VSC_SUPPORT),YES)
LDLIB_DYNAMIC += -lalislconaxvsc 
endif


ifneq ($(BUILDROOT_DISABLE_AUI_GFX_API),y)
LDLIB_DYNAMIC += \
	-ldirect \
	-ldirectfb \
	-lfusion
endif
ifeq ($(AUI_COMPILE_ITEM_MP_SUPPORT),YES)
LDLIB_DYNAMIC += -lnmpgoplayer
endif

ifeq ($(AUI_PACKAGE_ALIPLATFORM_INPUT),y)
LDLIB_DYNAMIC += -lalislinput
endif

ifeq ($(AUI_COMPILE_ITEM_PVR_SUPPORT),YES)
LDLIB_DYNAMIC += -lalichunk -lspvr -lalic -lalirsa -lbase -lalifs -lhld-sl
endif

DEPS := $(PROJECT_ALL_OBJS:.o=.d)
-include $(DEPS)

%.o : %.c
	@echo Compiling $@ ...
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -MM -MF dep $<
	@sed -e 's|.*:|$*.o:|' < dep > $*.d

clean_obj:
	rm dep; cd linux; rm -f *.d *.o
endif

# AUI library link
$(TARGET_LIB_NAME_STATIC): $(PROJECT_ALL_OBJS)
	$(MKDIR) $(AUI_DIR_OUTPUT)
	@$(AR) -rc $(TARGET_LIB_NAME_STATIC) $(PROJECT_ALL_OBJS)

$(TARGET_LIB_NAME_STATIC_SINGLE) : ALIPLATFORM_STATIC_LIB $(PROJECT_ALL_OBJS)
	$(MKDIR) $(TOPDIR)/output/linux/temp/
	cp $(TOPDIR)/src/linux/*.o $(TOPDIR)/output/linux/temp/
	@$(AR) -rc $(TARGET_LIB_NAME_STATIC_SINGLE) $(TOPDIR)/output/linux/temp/*.o
	
#Some function will not compile into libaui.so while using LDLIB_STATIC,thus Mini build don't pass .Using LDLIB_DYNAMIC enable libaui.so link to libalisl*.so and .*so in LDLIB_STATIC,make sure Mini build pass
$(TARGET_LIB_NAME_DYNAMIC): $(PROJECT_ALL_OBJS)
	@$(CC) $(LDFLAG) -o $(TARGET_LIB_NAME_DYNAMIC) $(PROJECT_ALL_OBJS) -Wall -shared -Wl,-soname,$(LIB_NAME_DYNAMIC) $(LDLIB_DYNAMIC)

$(TARGET_LIB_NAME_DYNAMIC_SINGLE): ALIPLATFORM_DYNAMIC_LIB $(PROJECT_ALL_OBJS)
	$(MKDIR) $(TOPDIR)/output/linux/temp2/
	cp $(TOPDIR)/src/linux/*.o $(TOPDIR)/output/linux/temp2/
	@$(CC) $(LDFLAG) -o $(TARGET_LIB_NAME_DYNAMIC_SINGLE) $(TOPDIR)/output/linux/temp2/*.o -Wall -shared -Wl,-soname,$(LIB_NAME_DYNAMIC_SINGLE)

ifeq ($(PLATFORM),TDS)
all: $(TARGET_LIB_NAME_STATIC)
else
all: $(TARGET_LIB_NAME_STATIC) $(TARGET_LIB_NAME_STATIC_SINGLE) $(TARGET_LIB_NAME_DYNAMIC) 
#$(TARGET_LIB_NAME_DYNAMIC_SINGLE) #don't need this target now.
endif

install: all

clean: clean_obj
	-rm -fr $(TARGET_LIB_NAME_STATIC) $(TARGET_LIB_NAME_STATIC_SINGLE) $(TARGET_LIB_NAME_DYNAMIC) $(TARGET_LIB_NAME_DYNAMIC_SINGLE)
	-rm -fr $(TOPDIR)/output/linux/temp/*
	-rm -fr $(TOPDIR)/output/linux/temp2/*

.PHONY: all install clean aui
