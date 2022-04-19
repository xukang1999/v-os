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
export RANLIB =ranlib

ifdef USER_MAKEFILE_NAME
  export MAKEFILE = $(USER_MAKEFILE_NAME)
else
  export MAKEFILE = Makefile
endif # USER_MAKEFILE_NAME

ifneq (,$(USER_DEPTH))
SRC_ROOT = $(USER_DEPTH)
else
	@echo "Please Define USER_DEPTH"
endif

LIB_DIR = $(SRC_ROOT)/target
DLL_DIR = $(SRC_ROOT)/target
BIN_DIR = $(SRC_ROOT)/bin


# Let users override the default output directories
ifeq (0,$(static))
__OBJDIR_PRI = shared
else
__OBJDIR_PRI = static
endif

ifeq (0,$(debug))
OBJDIR_PRI = $(__OBJDIR_PRI)/release
else
OBJDIR_PRI = $(__OBJDIR_PRI)/debug
endif

TARGET_PATH=$(SRC_ROOT)/target/objects/$(OBJDIR_PRI)
