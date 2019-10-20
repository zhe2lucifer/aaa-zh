# BOARD_CFG_M3733 BOARD_CFG_M3823 BOARD_CFG_M3515
#DEMO_BOARD = BOARD_CFG_M3733
#DEMO_BOARD = BOARD_CFG_M3823
DEMO_BOARD = BOARD_CFG_M3627

#############  compiler configure  ############
AUI_DIR_ROOT = ..
CFLAGS = -O1 -fPIC -fsigned-char -fno-inline-small-functions -Wall -I$(AUI_DIR_ROOT)/inc \
		-DAUI_LINUX -D_RD_DEBUG_ -D$(DEMO_BOARD) -DLNB_CFG_C_BAND

LDFLAGS = -L$(AUI_DIR_ROOT)/output/linux -laui_static

ifeq ($(DEMO_BOARD),BOARD_CFG_M3733)
CC = $(AUI_DIR_ROOT)/../../host/usr/bin/arm-none-linux-gnueabi-gcc
endif
ifeq ($(DEMO_BOARD),BOARD_CFG_M3823)
CC = $(AUI_DIR_ROOT)/../../host/usr/bin/mips-linux-gnu-gcc
endif
ifeq ($(DEMO_BOARD),BOARD_CFG_M3515)
CC = $(AUI_DIR_ROOT)/../../host/usr/bin/mips-linux-gnu-gcc
endif
ifeq ($(DEMO_BOARD),BOARD_CFG_M3627)
CC = $(AUI_DIR_ROOT)/../../host/usr/bin/mips-linux-gnu-gcc
endif


TAGERT = rec_ca rec
AUI_TEST_SRC = aui_stream_record_test_configure_dsc.c
#AUI_OBJS := $(patsubst %.c,%.o,$(AUI_TEST_SRC))
all: $(TAGERT)
rec_ca:  aui_stream_record_test_configure_dsc.c
	$(CC) aui_stream_record_test_configure_dsc.c $(CFLAGS) -lpthread $(LDFLAGS) -o ap_rec_ca
rec:  aui_stream_record_test.c
	$(CC) aui_stream_record_test.c $(CFLAGS) -lpthread $(LDFLAGS) -o ap_rb
#$(TAGERT):$(AUI_OBJS)
#	$(CC) $(AUI_OBJS)  -lpthread $(LDFLAGS) -o $@

#.c.o: $<
#	$(CC) $(CFLAGS) -c $<

.PHONY: all clean $(TAGERT)

clean:
	-rm -rf *.o
	-rm -rf $(TAGERT)

