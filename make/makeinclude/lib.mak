#
# Copyright (C) 2016-2020 南京北星极网络科技有限公司
#

include $(SRC_ROOT)/make/makeinclude/macros.mak

ifdef USER_LIB

ifeq (0,$(debug))
BASE_LIB_NAME = lib$(USER_LIB)_$(ARCH)_release
else
BASE_LIB_NAME = lib$(USER_LIB)_$(ARCH)_debug
endif

ifeq (0, $(static))
DLL = $(BASE_LIB_NAME).$(DLLEXT)
else
LIB = $(BASE_LIB_NAME).$(LIBEXT)
endif

endif # USER_LIB

ifdef LIB
  VLIB = $(LIB)
  CLEANUP += $(VLIB)
endif

ifdef DLL
  VDLL = $(DLL)
  CLEANUP += $(VDLL)
endif

#OBJDIR := $(OBJDIR)/$(USER_LIB)
.PHONY: all clean LibDirCheck

all: $(mkall) $(VLIB) $(VDLL)
	@echo 'OBJDIR' $(OBJDIR)
	@echo 'VLIB' $(VLIB)
	@echo 'VDLL' $(VDLL)
	@echo 'Objs' $(Objs)
	@echo 'SrcsObjs' $(SrcsObjs)
	@echo $(mkall)
	@echo 'make all over ..............................'

$(VLIB): LibDirCheck $(Objs)
	@if [ -f $(OBJDIR)/$@.obj ]; then rm $(OBJDIR)/$@.obj; fi
	@echo 'objs' $(Objs)
	@echo 'srcsobjs' $(SrcsObjs)
	@echo 'LD' $(LD)
	$(LD)  -r -o $(OBJDIR)/$@.obj $(SrcsObjs) $(ModulesObjs) $(USER_LINK_LIBS)
	$(AR) $(ARFLAGS) $(OBJDIR)/$@ $(OBJDIR)/$@.obj 
	chmod a+r $(OBJDIR)/$@
	$(RANLIB) $(OBJDIR)/$@
	@echo $(OBJDIR)/$@
	@echo $(LIB_DIR)
	@echo $(OBJDIR)
	rm -rf $(OBJDIR)/$@.obj
	@if [ ! -f $(LIB_DIR)/lib$(USER_LIB).$(LIBEXT) ]; \
	then ln -s $(OBJDIR)/$@ $(LIB_DIR)/lib$(USER_LIB).$(LIBEXT);\
	else unlink $(LIB_DIR)/lib$(USER_LIB).$(LIBEXT); ln -s $(OBJDIR)/$@ $(LIB_DIR)/lib$(USER_LIB).$(LIBEXT);fi
	@echo done!!!!!!!
	

$(VDLL): LibDirCheck $(Objs)
	@if [ -f $(OBJDIR)/$@.obj ]; then rm $(OBJDIR)/$@.obj; fi
	$(LD) -r -o $(OBJDIR)/$@.obj $(SrcsObjs) $(ModulesObjs)
	$(CC) $(SHARED_LINK_FLAGS) $(SHARED_LINK_RPATH) -o $(OBJDIR)/$@ $(OBJDIR)/$@.obj -lstdc++ $(USER_LINK_FLAGS) -L$(USER_DEPTH)/target $(USER_LINK_DLLS)
	@echo $(OBJDIR)/$@
	@echo $(LIB_DIR)
	@echo $(OBJDIR)
	rm -rf $(OBJDIR)/$@.obj
	@if [ ! -f $(LIB_DIR)/lib$(USER_LIB).$(DLLEXT) ]; \
	then ln -s $(OBJDIR)/$@ $(LIB_DIR)/lib$(USER_LIB).$(DLLEXT); \
	else unlink $(LIB_DIR)/lib$(USER_LIB).$(DLLEXT);ln -s $(OBJDIR)/$@ $(LIB_DIR)/lib$(USER_LIB).$(DLLEXT);fi
	@echo done!

clean: clean.local clean.obj
clean.local:
ifneq ($(CLEANUP),)
	$(RM) $(CLEANUP)
endif # CLEANUP

clean.obj:
	$(RM) $(OBJDIR)
	
LibDirCheck:
	@test -d $(OBJDIR) || mkdir -p $(OBJDIR)
	
include $(SRC_ROOT)/make/makeinclude/rules_compile.mak