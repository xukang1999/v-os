#
# Copyright (C) 2016-2020 南京北星极网络科技有限公司
#

include $(SRC_ROOT)/make/makeinclude/macros.mak

ifdef USER_BIN
BIN = $(USER_BIN)
endif # USER_BIN

ifdef BIN
  VBIN = $(BIN_DIR)/$(BIN)
  CLEANUP += $(VBIN)
  CLEANUP +=$(SrcsObjs) $(ModulesObjs)
endif

#OBJDIR := $(OBJDIR)/$(USER_BIN)

.PHONY: all clean clean.local clean.obj 

all: $(mkall) $(VBIN)
	@echo $(OBJDIR)
	@echo $(VBIN)
	@echo $(Objs)
	@echo $(SrcsObjs)
	@echo $(mkall)
	@echo 'make all over ..............................'
##$(LINK.cpp) $(CC_OUTPUT_FLAG) $@ $(SrcsObjs) $(ModulesObjs) $(USER_LINK_LIBS)
$(VBIN): BINDirCheck $(Objs) $(USER_LINK_LIBS)
	@echo 'build bin..............................'
	$(CXX) $(CC_OUTPUT_FLAG) $@ $(SrcsObjs) $(ModulesObjs) $(USER_LINK_LIBS) $(LINK_FLAGS) $(LDFLAGS)
	@echo 'done!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

clean: clean.local clean.obj
clean.local:
ifneq ($(CLEANUP),)
	$(RM) $(CLEANUP)
endif # CLEANUP

clean.obj:
	$(RM) $(OBJDIR)
BINDirCheck:
	@test -d $(OBJDIR) || mkdir -p $(OBJDIR)
include $(SRC_ROOT)/make/makeinclude/rules_compile.mak