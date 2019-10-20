
TESTS = aui_nim_test aui_tsi_test aui_tsg_test aui_nim_tsi_dmx_test
#  aui_dsc_test aui_smc_test aui_kl_test

all: $(TESTS)

aui_nim_test: aui_nim_test.o libaui
	$(CC) $< $(LDFLAGS) -o $@
	-cp $@ $(ROOTFS_DIR)/home/

aui_dsc_test: aui_dsc_test.o libaui
	$(CC) $< $(LDFLAGS) -o $@
	-cp $@ $(ROOTFS_DIR)/home/

aui_smc_test: aui_smc_test.o libaui
	$(CC) $< $(LDFLAGS) -o $@
	cp $@ $(ROOTFS_DIR)/home/

aui_kl_test: aui_kl_test.o libaui
	$(CC) $< $(LDFLAGS) -o $@
	cp $@ $(ROOTFS_DIR)/home/

aui_tsi_test: aui_tsi_test.o libaui
	$(CC) $< $(LDFLAGS) -o $@
	cp $@ $(ROOTFS_DIR)/home/

aui_tsg_test: aui_tsg_test.o libaui
	$(CC) $< $(LDFLAGS) -o $@
	cp $@ $(ROOTFS_DIR)/home/

aui_nim_tsi_dmx_test: aui_nim_tsi_dmx_test.o libaui
	$(CC) $< $(LDFLAGS) -o $@
	cp $@ $(ROOTFS_DIR)/home/

AUI_DIR_ROOT = ..
ROOT_DIR = $(AUI_DIR_ROOT)/..
ALIPLATFORM_DIR = $(ROOT_DIR)/aliplatform
ROOTFS_DIR = $(ROOT_DIR)/images/fs/rootfs

CFLAGS = -g -EL -mips32 -O1 -fPIC -fsigned-char -fno-inline-small-functions -Wall \
	-I$(AUI_DIR_ROOT)/inc \
	-I$(AUI_DIR_ROOT)/src/linux \
	-I$(ROOT_DIR)/platform/inc \
	-I$(ROOT_DIR)/kernel/linux/include/ali_common

CFLAGS += -I$(ALIPLATFORM_DIR)/include \
	-I$(ALIPLATFORM_DIR)/lib/libalisldmx/include \
	-I$(ALIPLATFORM_DIR)/lib/libalislvdec/include \
	-I$(ALIPLATFORM_DIR)/lib/libalislsnd/include \
	-I$(ALIPLATFORM_DIR)/lib/libalisldis/include

LDFLAGS = -L$(AUI_DIR_ROOT)/output/linux -laui \
	$(ALIPLATFORM_DIR)/src/.libs/libaliplatform.so \
	$(ALIPLATFORM_DIR)/lib/libalislnim/src/.libs/libalislnim.so \
	$(ALIPLATFORM_DIR)/lib/libalislsmc/src/.libs/libalislsmc.so \
	$(ALIPLATFORM_DIR)/lib/libalisldsc/src/.libs/libalisldsc.so \
	$(ALIPLATFORM_DIR)/lib/libalislce/src/.libs/libalislce.so \
	$(ALIPLATFORM_DIR)/lib/libalisltsi/src/.libs/libalisltsi.so \
	$(ALIPLATFORM_DIR)/lib/libalisltsg/src/.libs/libalisltsg.so \
	$(ALIPLATFORM_DIR)/lib/libalisldmx/src/.libs/libalisldmx.so \
	$(ALIPLATFORM_DIR)/lib/libalislsnd/src/.libs/libalislsnd.so \
	$(ALIPLATFORM_DIR)/lib/libalisldis/src/.libs/libalisldis.so \
	$(ALIPLATFORM_DIR)/lib/libalislvdec/src/.libs/libalislvdec.so

libaui:
	cd $(AUI_DIR_ROOT)/src/linux; $(MAKE) FLATFORM=LINUX -f ../makefile -I../../
libaui-clean:
	cd $(AUI_DIR_ROOT)/src/linux; $(MAKE) FLATFORM=LINUX -f ../makefile -I../../ clean

libaui-samples:
	cd $(AUI_DIR_ROOT)/samples; $(MAKE) FLATFORM=LINUX -f Makefile_last -I..
libaui-samples-clean:
	cd $(AUI_DIR_ROOT)/samples; $(MAKE) FLATFORM=LINUX -f Makefile_last  -I.. clean



aliplatform:
	cd $(ROOT_DIR)/aliplatform; make; make install

CC = /home/tools/mipsel-4.4/bin/mipsel-linux-gnu-gcc

.c.o: $<
	$(CC) $(CFLAGS) -c $<

.PHONY: all clean $(TESTS)

clean: libaui-clean
	-rm -rf *.o
	-rm -rf $(TESTS)

