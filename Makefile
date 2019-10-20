TOPDIR = .

include $(TOPDIR)/aui.cfg

SUBDIRS = src

ifeq ($(PLATFORM),TDS)
	SUBDIRS += samples
	ifdef _TEST_OLD_CI_
		SUBDIRS += nestor/targetapp
	endif
endif

ifeq ($(PLATFORM),LINUX)
	ifeq ($(BUILDROOT_BUILD_AUI_SAMPLES),y)
		SUBDIRS += samples
	endif
	ifeq ($(BUILDROOT_BUILD_AUI_NESTOR),y)
		SUBDIRS += nestor
	endif
	ifeq ($(BUILDROOT_BUILD_AUI_SFU_TEST),y)
		SUBDIRS += sfu_test
	endif
endif


all: $(SUBDIRS)

aui:
	@$(MAKE) -C ./src

aui_clean:
	@$(MAKE) -C ./src clean

aui_test:
	@$(MAKE) -C ./samples

aui_test_clean:
	@$(MAKE) -C ./samples clean

aui_nestor:
	@$(MAKE) -C ./nestor/targetapp

aui_nestor_clean:
	@$(MAKE) -C ./nestor/targetapp  clean

ifeq ($(PLATFORM),TDS)
clean: 
	@for subdir in ${SUBDIRS};\
		do ${MAKE} -C $${subdir} clean ;done
endif

ifeq ($(PLATFORM),LINUX)
sfu_test:
	@$(MAKE) -C ./sfu_test
endif

# For buildroot
$(SUBDIRS):
ifeq ($(PLATFORM),LINUX)
	@$(MAKE) -C $@ -f build.mk
endif
ifeq ($(PLATFORM),TDS)
	@$(MAKE) -C $@
endif


.PHONY: all clean $(SUBDIRS)
