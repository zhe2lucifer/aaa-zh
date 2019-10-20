
TESTS = aui_trick_play_test

all: $(TESTS)

aui_trick_play_test: aui_trick_play_test.o
	$(CC) $< $(LDFLAGS) -lpthread -o $@
	-cp $@ $(APP_INSTALL_DIR)/usr/bin/

AUI_DIR_ROOT = ..
APP_INSTALL_DIR = $(AUI_DIR_ROOT)/../../target


CFLAGS = -O1 -fPIC -fsigned-char -fno-inline-small-functions -Wall -I$(AUI_DIR_ROOT)/inc

LDFLAGS = $(AUI_DIR_ROOT)/output/linux/libaui_static.a

CC = /opt/arm-2013.11/bin/arm-none-linux-gnueabi-gcc

.c.o: $<
	$(CC) $(CFLAGS) -c $<

.PHONY: all clean $(TESTS)

ap: clean
	make -f Makefile.section
	make -f Makefile.stream
	-cp ap_s* $(APP_INSTALL_DIR)/usr/bin/

clean:
	-rm -rf *.o
	-rm -rf ap_s*
	-rm -rf $(TESTS)

