#
# Copyright (C) 2016-2020 南京北星极网络科技有限公司
#
debug ?= 1
optimize ?= 1
#profile ?= 1
exceptions ?= 1
threads ?= 0
static ?=1

OBJEXT ?= o
LIBEXT ?= a
DLLEXT ?= so
CC_OUTPUT_FLAG ?= -o
DLL_OUTPUT_FLAG ?= -o
LINK_OUTPUT_FLAG ?= -o
ARCH:=$(shell uname -m)

ifeq (,$(debug))
  debug = 1
endif
ifeq (,$(optimize))
  optimize = 0
endif

ifeq (,$(_MACHINE))
	DEFFLAGS += -D_CPU_TYPE=$(_CPU_TYPE) -D$(_CPU_TYPE) -DCPU=$(_CPU)
	ASFLAGS += -D_CPU_TYPE=$(_CPU_TYPE) -D$(_CPU_TYPE) -DOS_LINUX_PC -DCPU=$(_CPU) 
else
	DEFFLAGS += -$(_MACHINE) -D_CPU_TYPE=$(_CPU_TYPE) -D$(_CPU_TYPE) 
	ASFLAGS += -$(_MACHINE) -D_CPU_TYPE=$(_CPU_TYPE) -D$(_CPU_TYPE)  
endif

WERROR=-Wall\
-Wno-pointer-sign -Wmissing-include-dirs -fdiagnostics-show-option \
-Wno-array-bounds\
-Wno-shift-count-negative\
-Wno-shift-count-overflow\
-Wno-unused-value\
-Wno-format-security\
-Wno-unused-variable\
-Wno-cast-qual

ifeq (1,$(debug))
	DEFFLAGS += -DDEBUG_VERSION
	DEFFLAGS += -DCAW_DEBUG
	DEFFLAGS +=-pipe -fsigned-char -g $(WERROR)
	ASFLAGS +=-pipe  -fsigned-char -g -P -x assembler-with-cpp -fno-threadsafe-statics $(WERROR)
else
	DEFFLAGS += -DRELEASE_VERSION
	DEFFLAGS += -DCAW_DISABLE_TRACE
	DEFFLAGS +=-fno-builtin  -pipe  -O3 -fno-strict-aliasing -fsigned-char -fno-omit-frame-pointer $(WERROR)
	ASFLAGS +=-fno-builtin  -pipe  -O3 -fno-strict-aliasing -fsigned-char -fno-omit-frame-pointer -P -x assembler-with-cpp $(WERROR)
endif


ifeq ($(threads),1)
  DEFFLAGS += -D_REENTRANT
endif # threads

ifeq ($(static),0)
DEFFLAGS +=-fPIC 
ASFLAGS +=-fPIC
endif


INCLDIRS +=
LIBPATH +=
DLLSINCLUDE +=


USER_LINK_FLAGS +=$(LIBPATH)
ASFLAGS +=

INCLDIRS += $(USER_INCLDIRS)
ASMINCLDIRS += $(USER_ASM_INCLDIRS)	
C_INCLDIRS += $(USER_C_INCLDIRS)		
DEFFLAGS += $(USER_DEFFLAGS) 
LIBS += $(USER_LINK_LIBS)
DLLS += $(DLLSINCLUDE) $(USER_LINK_DLLS)


CPPFLAGS += -Wno-deprecated -std=c++11 $(DEFFLAGS) $(USER_C++_DEFFLAG) $(INCLDIRS)
CFLAGS += -std=c99 $(DEFFLAGS) $(USER_C_DEFFLAGS) $(C_INCLDIRS) 
LDFLAGS += $(DLLSINCLUDE) $(USER_LINK_DLLS)
DEFLINKFLAGS +=
LDFLAGS+= $(DEFLINKFLAGS)
#ASFLAGS +=$(DEFFLAGS)
NASMFLAG += $(ASFLAGS) $(ASMINCLDIRS)

LINK_FLAGS += $(USER_LINK_FLAGS)

COMPILE.c  = $(CC) $(CFLAGS)
COMPILE.cc  = $(CXX) $(CPPFLAGS)
COMPILE.cpp = $(CXX) $(CPPFLAGS)
COMPILE.cxx = $(CXX) $(CPPFLAGS)
COMPILE.as = $(CC) $(NASMFLAG)

COMPILE.m  = $(CC) $(CFLAGS)
COMPILE.mm  = $(CXX) $(CPPFLAGS)

LINK.c  = $(CC) $(LINK_FLAGS) $(LDFLAGS) 
LINK.cc  = $(CXX) $(LINK_FLAGS) $(LDFLAGS) 
LINK.m  = $(CC) $(LINK_FLAGS) $(LDFLAGS) 
LINK.mm  = $(CXX) $(LINK_FLAGS) $(LDFLAGS) 
LINK.cpp = $(CXX) $(LINK_FLAGS) $(LDFLAGS)
LINK.cxx = $(CXX) $(LINK_FLAGS) $(LDFLAGS)

NULL_STDERR = 2>$(/dev/null) || true

