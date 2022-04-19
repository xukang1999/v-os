#
# Copyright (C) 2013, Nanjing WavPhone Network Technology Co., Ltd
# Author yijian, walkinheart@gmail.com
#


include $(SRC_ROOT)/make/makeinclude/platform.mak


#----------------------------------------------------------------------------
#      Platform-independent macro definitions
#----------------------------------------------------------------------------

####
#### Defaults are exceptions off (0), fast off (0), rtti off (0), and
#### versioned_so on (1).
####
exceptions ?= 0
fast ?= 0
rtti ?= 0

ifdef USER_OBJ_DIR
OBJDIR = $(USER_OBJ_DIR)
endif

#VPATH=$(USER_MODULE_DIR):$(USER_MODULE_DIR)


ifdef USER_SRCS_s 
Srcs_s_Objs   = $(patsubst %.s,%.o,$(notdir $(USER_SRCS_s)))
Srcs_s_path   = $(strip $(dir $(USER_SRCS_s)))
endif

ifdef USER_SRCS_S 
Srcs_S_Objs   = $(patsubst %.S,%.o,$(notdir $(USER_SRCS_S)))
Srcs_S_path   = $(strip $(dir $(USER_SRCS_S)))
endif

ifdef USER_SRCS_C
Srcs_c_path   = $(strip $(dir $(USER_SRCS_C)))
Deps_c        = $(patsubst %.c,%.cd,$(notdir $(USER_SRCS_C)))
Srcs_c_Objs   = $(patsubst %.cd,%.o,$(Deps_c))
endif

ifdef USER_SRCS_CPP
Srcs_cpp_path   = $(strip $(dir $(USER_SRCS_CPP)))
Deps_cpp      = $(patsubst %.cpp,%.cppd,$(notdir $(USER_SRCS_CPP)))
Srcs_cpp_Objs   = $(patsubst %.cppd,%.o,$(Deps_cpp))
endif

ifdef USER_SRCS_CXX
Srcs_cxx_path   = $(strip $(dir $(USER_SRCS_CXX)))
Deps_cxx      = $(patsubst %.cxx,%.cxxd,$(notdir $(USER_SRCS_CXX)))
Srcs_cxx_Objs   = $(patsubst %.cxxd,%.o,$(Deps_cxx))
endif

ifdef USER_SRCS_CC
Srcs_cc_path   = $(strip $(dir $(USER_SRCS_CC)))
Deps_cc      	= $(patsubst %.cc,%.ccd,$(notdir $(USER_SRCS_CC)))
Srcs_cc_Objs   = $(patsubst %.ccd,%.o,$(Deps_cc))
endif

ifdef USER_SRCS_M
Srcs_m_path   = $(strip $(dir $(USER_SRCS_M)))
Deps_m        = $(patsubst %.m,%.md,$(notdir $(USER_SRCS_M)))
Srcs_m_Objs   = $(patsubst %.md,%.o,$(Deps_m))
endif

ifdef USER_SRCS_MM
Srcs_mm_path   = $(strip $(dir $(USER_SRCS_MM)))
Deps_mm      	= $(patsubst %.mm,%.mmd,$(notdir $(USER_SRCS_MM)))
Srcs_mm_Objs   = $(patsubst %.mmd,%.o,$(Deps_mm))
endif

SrcsObjs_base= $(Srcs_s_Objs) $(Srcs_S_Objs) $(Srcs_c_Objs) $(Srcs_cpp_Objs) $(Srcs_cxx_Objs) $(Srcs_cc_Objs) $(Srcs_m_Objs) $(Srcs_mm_Objs)

SrcsObjs = $(foreach OBJ,$(SrcsObjs_base), $(OBJDIR)/$(OBJ))

export ALLOBJ += $(SrcsObjs)
export OPER_DIR += $(USER_MODULE)

ModulesObjs = $(foreach module,$(USER_SUBDIRS), $(OBJDIR)/$(module)/$(module).mo)
Objs        = $(SrcsObjs) $(ModulesObjs)

mkall       = $(foreach module,$(USER_SUBDIRS), $(OBJDIR)/$(module)/$(module).moduleall)
mkclean     = $(addsuffix .clean,$(USER_SUBDIRS))

null:=

space =$(null) $(null)

CODE_PATH           = $(Srcs_s_path)  $(Srcs_S_path) $(Srcs_c_path) $(Srcs_cc_path) $(Srcs_cpp_path) $(Srcs_cxx_path) $(Srcs_m_path) $(Srcs_mm_path)

vpath %.s       $(subst $(space),:,$(strip $(CODE_PATH) ))
vpath %.S       $(subst $(space),:,$(strip $(CODE_PATH) ))
vpath %.c       $(subst $(space),:,$(strip $(CODE_PATH) ))
vpath %.cpp     $(subst $(space),:,$(strip $(CODE_PATH) ))
vpath %.h       $(subst $(space),:,$(strip $(CODE_PATH) ))
vpath %.hxx     $(subst $(space),:,$(strip $(CODE_PATH) ))
vpath %.hpp     $(subst $(space),:,$(strip $(CODE_PATH) ))
vpath %.cxx     $(subst $(space),:,$(strip $(CODE_PATH) ))
vpath %.cc      $(subst $(space),:,$(strip $(CODE_PATH) ))
vpath %.m       $(subst $(space),:,$(strip $(CODE_PATH) ))
vpath %.mm      $(subst $(space),:,$(strip $(CODE_PATH) ))
