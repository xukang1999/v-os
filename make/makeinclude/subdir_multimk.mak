#
# Copyright (C) 2016-2020 南京北星极网络科技有限公司
#

include $(SRC_ROOT)/make/makeinclude/macros.mak

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

all: all.nested
	
clean: clean.nested
	
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

include $(SRC_ROOT)/make/makeinclude/rules_compile.mak