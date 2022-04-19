#
# Copyright (C) 2016-2017, Nanjing StarOS Technology Co., Ltd
#
build_ace=1
build_unittest=0

ifeq (,$(os))
os=linux
endif

ifeq (0,$(debug))
ifeq (0,$(static))
TARNAME=ace_release_shared_$(os)_armv7_v7.0.2
INSTALLNAME=install.sh
else
TARNAME=ace_release_static_$(os)_armv7_v7.0.2
INSTALLNAME=install.sh
endif
else
ifeq (0,$(static))
TARNAME=ace_debug_shared_$(os)_armv7_v7.0.2
INSTALLNAME=install.sh
else
TARNAME=ace_debug_static_$(os)_armv7_v7.0.2
INSTALLNAME=install.sh
endif
endif


ifeq (1,$(build_ace))
buidlall +=ace
tocleanall +=aceclean
endif

ifeq (1,$(build_unittest))
buidlall +=unittest
tocleanall +=unittestclean
endif

all: check_dir $(buidlall) 
	echo $(buidlall)
	cp -rf $(ROOTDIR)/docs/* $(ROOTDIR)/build/linux/armv7/$(TARNAME)/docs/
	cp $(ROOTDIR)/COPYING $(ROOTDIR)/build/linux/armv7/$(TARNAME)/
	cp $(ROOTDIR)/AUTHORS $(ROOTDIR)/build/linux/armv7/$(TARNAME)/
	cp $(ROOTDIR)/NEWS $(ROOTDIR)/build/linux/armv7/$(TARNAME)/
	cp $(ROOTDIR)/README $(ROOTDIR)/build/linux/armv7/$(TARNAME)/
	cp $(ROOTDIR)/VERSION.txt $(ROOTDIR)/build/linux/armv7/$(TARNAME)/
	cp $(ROOTDIR)/THANKS $(ROOTDIR)/build/linux/armv7/$(TARNAME)/
	mv $(TARNAME) tmp
	tar zcvf $(TARNAME).tar.gz -C $(ROOTDIR)/build/linux/armv7/ tmp
	cat $(INSTALLNAME) $(TARNAME).tar.gz > $(TARNAME).bin 
	@echo "build" $(buidlall)
	rm -rf tmp

.PHONY:ace
ace: check_dir
	chmod a+x $(ACE_ROOT)/make/add_rel_link.sh
ifeq (0,$(static))
	make -C $(ACE_ROOT) ssl=0 static=0 static_libs_only=0
	rm -rf $(ACE_LIB_PATH)/*.a
	cp -rLf $(ACE_ROOT)/libs/* $(ROOTDIR)/build/linux/armv7/$(TARNAME)/libs/
else
	make -C $(ACE_ROOT) ssl=0 static=1 static_libs_only=1
	rm -rf $(ACE_LIB_PATH)/*.so
	cp $(ACE_ROOT)/libs/libACE.a $(ROOTDIR)/build/linux/armv7/$(TARNAME)/libs/
endif
	cp -rf $(ACE_ROOT)/include/ace/* $(ROOTDIR)/build/linux/armv7/$(TARNAME)/include/ace/
.PHONY:unittest
unittest:check_dir
	make -C $(UNITTEST_ROOT) static=$(static)
	cp -rf $(UNITTEST_ROOT)/bin/* $(ROOTDIR)/build/linux/armv7/$(TARNAME)/bin/
check_dir: 
	@test -d $(ROOTDIR)/build/linux/armv7/$(TARNAME) || mkdir -p $(ROOTDIR)/build/linux/armv7/$(TARNAME)
	@test -d $(ROOTDIR)/build/linux/armv7/$(TARNAME)/libs || mkdir -p $(ROOTDIR)/build/linux/armv7/$(TARNAME)/libs
	@test -d $(ROOTDIR)/build/linux/armv7/$(TARNAME)/docs || mkdir -p $(ROOTDIR)/build/linux/armv7/$(TARNAME)/docs
	@test -d $(ROOTDIR)/build/linux/armv7/$(TARNAME)/include/ace || mkdir -p $(ROOTDIR)/build/linux/armv7/$(TARNAME)/include/ace
#################################################################################
	
cleanall: $(tocleanall)
.PHONY: aceclean
aceclean:
	make aceclean -C $(ACE_ROOT)
