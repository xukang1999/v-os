#
# Copyright (C) 2016-2020 南京北星极网络科技有限公司
#

export RM              = rm -rf
export RMDIR         = rm -rf
export MKDIR         = mkdir -p
export MOVE          = mv -f
export ECHO          = echo
export CP              = cp -f
export MAKE          = make
export LN            = ln -sf
export PWD            = pwd

export MAKEFILE = Makefile


ifdef USER_DEPTH
  SRC_ROOT = $(USER_DEPTH)
else
  all clean: echo_error
endif # USER_DEPTH

ifdef USER_KMODULE_NAME
  include $(SRC_ROOT)/make/makeinclude/kmodule.mak
else ifdef USER_MODULE
  include $(SRC_ROOT)/make/makeinclude/module.mak
else ifdef USER_LIB
  include $(SRC_ROOT)/make/makeinclude/lib.mak
else ifdef USER_BIN
  include $(SRC_ROOT)/make/makeinclude/bin.mak
else ifdef USER_SUB_DIRS
  include $(SRC_ROOT)/make/makeinclude/subdir_multimk.mak
else ifdef USER_MULTI_MAKEFILES
  include $(SRC_ROOT)/make/makeinclude/subdir_multimk.mak
else
  all clean: lib_error
endif

echo_error:
	@echo "Error, Makefile must define USER_DEPTH."
	
lib_error:
	@echo $(USER_LIB)
	@echo "Error, must define someting."

