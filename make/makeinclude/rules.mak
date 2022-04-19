#
# Copyright (C) 2016-2020 南京北星极网络科技有限公司
#

LIB ?=
DLL ?=
CLEANUP ?=

OCFLAGS += -O3
PIC     = -fPIC
ARFLAGS = rv

#ARFLAGS = rcs
RM      = rm -rf


ifdef USER_SRCS
  OBJS += $(addsuffix .$(OBJEXT),$(addprefix $(OBJDIR)/,$(basename  $(USER_SRCS))))
  CLEANUP += $(OBJS)
endif # USER_SRCS

ifdef LIB
  VLIB = $(LIB_DIR)/$(LIB)
  CLEANUP += $(VLIB)
endif
ifdef BIN
  VBIN = $(BIN_DIR)/$(BIN)
  CLEANUP += $(VBIN)
endif
ifdef DLL
  VDLL = $(DLL_DIR)/$(DLL)
  CLEANUP += $(VDLL)
endif

# If the client makefile is not called "Makefile", the USER_MAKEFILE_NAME
# variable must be set to its actual name before including this
# file to allow the recursive MAKE to work properly.
ifdef USER_MAKEFILE_NAME
  MAKEFILE = $(USER_MAKEFILE_NAME)
else
  MAKEFILE = Makefile
endif # USER_MAKEFILE_NAME

MULTI_MAKEFILES =
ifdef USER_MULTI_MAKEFILES
  MULTI_MAKEFILES = $(addsuffix .mmakefiles, $(USER_MULTI_MAKEFILES))
endif # USER_MULTI_MAKEFILES

SUB_DIRS =
ifdef USER_SUB_DIRS
  SUB_DIRS = $(addsuffix .subdirs, $(USER_SUB_DIRS))
endif # USER_SUB_DIRS

TARGETS_LOCAL  = all.local clean.local
TARGETS_NESTED = $(TARGETS_LOCAL:.local=.nested)


all: all.nested all.local
	
clean: clean.nested clean.local

#all.local:
#	@echo $(OBJDIR)
#	@echo $(VBIN) $(VLIB) $(VDLL) $(OBJS)

all.local: check_dir $(VBIN) $(VLIB) $(VDLL) $(OBJS)

CHECK_DIRS += $(OBJDIR) $(LIB_DIR) $(BIN_DIR) $(DLL_DIR)
check_dir: 
#$(CHECK_DIRS)
#@echo $(CHECK_DIRS)	
	@test -d $(OBJDIR) || mkdir -p $(OBJDIR)
	@test -d $(LIB_DIR) || mkdir -p $(LIB_DIR)
	@test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	@test -d $(DLL_DIR) || mkdir -p $(DLL_DIR)
ifdef USER_SUBDIRS
	@for subdir in $(USER_SUBDIRS) ; do ( mkdir -p $(OBJDIR)/$$subdir ) ; done
endif

#$(CHECK_DIRS):
#	@echo objdir $(OBJDIR) libdir:$(LIB_DIR) bindir:$(BIN_DIR) dlldir:$(DLL_DIR)
#	@test -d $@ || mkdir -p $@ 

$(OBJDIR)/%.$(OBJEXT): %.m
	$(COMPILE.m) -ObjC -c $< -o $@

$(OBJDIR)/%.$(OBJEXT): %.mm
	$(COMPILE.mm) -ObjC++ -c $< -o $@
	
$(OBJDIR)/%.$(OBJEXT): %.c
	$(COMPILE.c) -c $< -o $@

$(OBJDIR)/%.$(OBJEXT): %.cc
	$(COMPILE.cc) -c $< -o $@

$(OBJDIR)/%.$(OBJEXT): %.cpp
	$(COMPILE.cpp) -c $< -o $@

$(OBJDIR)/%.$(OBJEXT): %.cxx
	$(COMPILE.cxx) -c $< -o $@

$(OBJDIR)/%.$(OBJEXT): %.s
	$(COMPILE.as) -c $< -o $@

$(OBJDIR)/%.$(OBJEXT): %.S
	$(COMPILE.as) -D__ASSEMBLY__ -c $< -o $@
	
# user can ar USER_LINK_LIBS into xxx.a file
$(VLIB): $(OBJS) $(USER_LINK_LIBS)
	@if [ -f $@.obj ]; then rm $@.obj; fi
	$(LD)  -r -o $@.obj $(OBJS) $(USER_LINK_LIBS)
	$(AR) $(ARFLAGS) $@ $@.obj 
	-chmod a+r $@
	$(RANLIB) $@
	@echo $@
#rm -rf $@.obj
	@echo $(LIB_DIR)
	@if [ ! -f $(LIB_DIR)/lib$(USER_LIB).$(LIBEXT) ]; then ln -s $@ $(LIB_DIR)/lib$(USER_LIB).$(LIBEXT); fi
$(VDLL): $(OBJS) $(USER_LINK_LIBS)
	@if [ -f $@.obj ]; then rm $@.obj; fi
	$(LD) --warn-common -r -o $@.obj $(OBJS)
	$(CC) -shared -o $@ $@.obj $(USER_LINK_LIBS)
	rm -rf $@.obj
	@if [ ! -f $(LIB_DIR)/lib$(USER_DLL).$(DLLEXT) ]; then ln -s $@ $(LIB_DIR)/lib$(USER_DLL).$(DLLEXT); fi
	@echo done!

# don't depend on $(LIBS) due to -L and -l
$(VBIN): $(OBJS) $(VLIB) $(LIBS)
	$(LINK.cpp) $(CC_OUTPUT_FLAG) $@ $(OBJS) $(VLIB) $(LIBS) $(DLLS)

clean.local:
ifneq ($(CLEANUP),)
	$(RM) $(CLEANUP)
endif # CLEANUP

%.mmakefiles: %
	$(MAKE) -f $< $(MMAKEFILES_TARGET)

%.subdirs: %
	$(MAKE) -C $< $(SUBDIRS_TARGET)

$(TARGETS_NESTED):
ifdef MULTI_MAKEFILES
	$(MAKE) -f $(MAKEFILE) MMAKEFILES_TARGET=$(@:.nested=) $(MULTI_MAKEFILES)
endif # MULTI_MAKEFILES
ifdef SUB_DIRS
	$(MAKE) -f $(MAKEFILE) SUBDIRS_TARGET=$(@:.nested=) $(SUB_DIRS)
endif # SUB_DIRS
