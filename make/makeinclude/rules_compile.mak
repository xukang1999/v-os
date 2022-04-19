#
# Copyright (C) 2016-2020 南京北星极网络科技有限公司
#

compiles:
	@echo 'csourcepath' $(Srcs_c_path)

#.PHONY: $(mkall) $(mkclean) all clean
$(mkall): check_dir
	@echo $(mkall)
	@echo 'hello mkall'
	@echo 'objdir:' $(OBJDIR)
	@echo 'objs:' $(Objs)
	@echo 'c:' $(USER_SRCS_C)
	@echo 'OBJEXT' $(OBJDIR)/$(OBJEXT)
	@echo 'SrcsObjs' $(SrcsObjs)
	@echo 'allobj' $(ALLOBJ)
	@echo 'OPER_DIR' $(OPER_DIR)
	#@if not exist $(basename $(notdir $@)) @mkdir -p $(basename $(notdir $@))
	@test -d $(OBJDIR)/$(basename $(notdir $@)) || mkdir -p $(OBJDIR)/$(basename $(notdir $@))
	$(MAKE) -f $(MAKEFILE) modules -C $(basename $(notdir $@))

$(mkclean):
	$(RM) -rf $(OBJDIR)/$(basename $@)

check_dir: 
	@echo $(OBJDIR)	
	@test -d $(OBJDIR) || mkdir -p $(OBJDIR)
	@test -d $(LIB_DIR) || mkdir -p $(LIB_DIR)
	@test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	@test -d $(DLL_DIR) || mkdir -p $(DLL_DIR)

ObjDirCheck:
	@test -d $(OBJDIR) || mkdir -p $(OBJDIR)
#################################
#本makefile编译入口all
#################################
modules:$(OBJDIR)/$(USER_MODULE).mo
	$(ECHO) [!][Check Module Obj] $(OPER_DIR).mo over

#$(ModName).mo:$(SrcsObjs) $(ModulesObjs)
$(OBJDIR)/$(USER_MODULE).mo: ObjDirCheck $(Objs)
	@echo $(OBJDIR)/$(USER_MODULE).mo
	@echo $(OBJDIR) 
	@echo 'moduleobjs' $(ModulesObjs)
	@echo 'srcsobjs' $(SrcsObjs)
	@echo 'allobj' $(ALLOBJ)
ifeq (, $(LINKLIB))
	$(ECHO) [Relinking Module Obj] $(USER_MODULE).mo....................
	$(LD) -r -o $(OBJDIR)/$(USER_MODULE).mo $(SrcsObjs) $(ModulesObjs)
else
	$(ECHO) [Add $(OPER_DIR) Objs ] ....................
	$(AR) crus $(LIBBASE) $(SrcsObjs)
	$(ECHO) [To] $(notdir $(LIBBASE)) OK!
endif
	
$(OBJDIR)/%.$(OBJEXT):%.s
	$(ECHO) [Compile Assemble in $(OPER_DIR)] Src file:$(notdir $<)...
	$(COMPILE.as) -D__ASSEMBLY__ -c $< -o $@
	$(ECHO) To      $(notdir $@) OK!
	$(ECHO) '.'

$(OBJDIR)/%.$(OBJEXT):%.S
	$(ECHO) [Compile Assemble in $(OPER_DIR)] Src file:$(notdir $<)...
	$(COMPILE.as) -D__ASSEMBLY__ -c $< -o $@
	$(ECHO) To      $(notdir $@) OK!
	$(ECHO) '.'

$(OBJDIR)/%.$(OBJEXT):%.c
	$(ECHO) [Compile Obj in $(OPER_DIR)] Src file:$(notdir $<)...
	$(COMPILE.c) -c $< -o $@
	$(ECHO) [To] $(notdir $@) OK!

$(OBJDIR)/%.$(OBJEXT):%.cpp
	$(ECHO) [Compile Obj in $(OPER_DIR)] Src file:$(notdir $<)...
	$(COMPILE.cpp) -c $< -o $@
	$(ECHO) [To] $(notdir $@) OK!
	
$(OBJDIR)/%.$(OBJEXT):%.cxx
	$(ECHO) [Compile Obj in $(OPER_DIR)] Src file:$(notdir $<)...
	$(COMPILE.cxx) -c $< -o $@
	$(ECHO) [To] $(notdir $@) OK!
	
$(OBJDIR)/%.$(OBJEXT):%.cc
	$(ECHO) [Compile Obj in $(OPER_DIR)] Src file:$(notdir $<)...
	$(COMPILE.cc) -c $< -o $@
	$(ECHO) [To] $(notdir $@) OK!

$(OBJDIR)/%.$(OBJEXT):%.m
	$(ECHO) [Compile Obj in $(OPER_DIR)] Src file:$(notdir $<)...
	$(COMPILE.m) -ObjC -c $< -o $@
	$(ECHO) [To] $(notdir $@) OK!
	
$(OBJDIR)/%.$(OBJEXT):%.mm
	$(ECHO) [Compile Obj in $(OPER_DIR)] Src file:$(notdir $<)...
	$(COMPILE.mm) -ObjC++ -c $< -o $@
	$(ECHO) [To] $(notdir $@) OK!
	
######################################################################

