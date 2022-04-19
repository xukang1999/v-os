#
# Copyright (C) 2016-2017, Nanjing StarOS Technology Co., Ltd
#
build_freebsd=1
VERSION=1.0.0
ifeq (,$(os))
os=linux
endif

ifeq (0,$(debug))
ifeq (0,$(static))
TARNAME=vos_release_shared_$(os)_x86_64_v$(VERSION)
INSTALLNAME=install.sh
else
TARNAME=vos_release_static_$(os)_x86_64_v$(VERSION)
INSTALLNAME=install.sh
endif
else
ifeq (0,$(static))
TARNAME=vos_debug_shared_$(os)_x86_64_v$(VERSION)
INSTALLNAME=install.sh
else
TARNAME=vos_debug_static_$(os)_x86_64_v$(VERSION)
INSTALLNAME=install.sh
endif
endif

export STARDLLBUILD=$(ROOTDIR)/build/linux/x86_64/$(TARNAME)

ifeq (1,$(build_freebsd))
buidlall +=freebsd
tocleanall +=freebsdclean
endif


all: check_dir $(buidlall) 
	echo $(buidlall)
	cp -rf $(ROOTDIR)/docs/* $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/docs/
	cp -rf $(ROOTDIR)/conf/* $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/conf/
	cp -rf $(ROOTDIR)/license $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/
	cp $(ROOTDIR)/COPYING $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/
	mv $(TARNAME) tmp
	tar zcvf $(TARNAME).tar.gz -C $(ROOTDIR)/build/linux/x86_64/ tmp
	cat $(INSTALLNAME) $(TARNAME).tar.gz > $(TARNAME).bin 
	@echo "build" $(buidlall)
	rm -rf tmp
.PHONY:freebsd
freebsd:check_dir
	make -C $(ROOTDIR)  static=$(static)
	cp -rf $(ROOTDIR)/src/freebsd/include/staros/* $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/include/staros/
	cp -rf $(ROOTDIR)/src/ports/include/ports/* $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/include/ports/
ifeq (0,$(static))
	cp -Lf $(ROOTDIR)/target/*.so $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/libs/
else
	cp -Lf $(ROOTDIR)/target/*.a $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/libs/
endif
check_dir: 
	@test -d $(ROOTDIR)/build/linux/x86_64/$(TARNAME) || mkdir -p $(ROOTDIR)/build/linux/x86_64/$(TARNAME)
	@test -d $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/bin || mkdir -p $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/bin
	@test -d $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/conf || mkdir -p $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/conf
	@test -d $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/logs || mkdir -p $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/logs
	@test -d $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/libs || mkdir -p $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/libs
	@test -d $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/docs || mkdir -p $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/docs
	@test -d $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/include || mkdir -p $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/include
	@test -d $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/include/staros || mkdir -p $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/include/staros
	@test -d $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/include/ports || mkdir -p $(ROOTDIR)/build/linux/x86_64/$(TARNAME)/include/ports
#################################################################################
	
cleanall: $(tocleanall)
###########################################################################################
.PHONY:freebsdclean
freebsdclean:
	make clean -C $(ROOTDIR)
	rm -rf $(ROOTDIR)/target/*
	
