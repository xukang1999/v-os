#
# Copyright (C) 2016-2020 南京北星极网络科技有限公司
#

include $(SRC_ROOT)/make/makeinclude/macros.mak

.PHONY: all clean

export ModulePath=

all: check_obj_dir modules
	$(ECHO) 'modules objs' $(Objs)
	@echo $(mkall)
	@echo "ModulePath" $(ModulePath)

$(ModulesObjs):$(mkall)
	$(ECHO) '.'

clean:$(mkclean)
	@echo $(OBJDIR)
	$(RM) $(OBJDIR)

check_obj_dir: 
#$(CHECK_DIRS)
	@echo 'obj' $(OBJDIR)	
	@test -d $(OBJDIR) || mkdir -p $(OBJDIR)
	@test -d $(LIB_DIR) || mkdir -p $(LIB_DIR)
	@test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	@test -d $(DLL_DIR) || mkdir -p $(DLL_DIR)
	

include $(SRC_ROOT)/make/makeinclude/rules_compile.mak
