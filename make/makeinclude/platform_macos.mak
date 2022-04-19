#
# Copyright (C) 2016-2022 南京北星极网络科技有限公司
#

DEFFLAGS += -DCAW_UNIX -DVOS_UNIX -DXP_MACOS -DCAW_DARWIN -DOS_MAC_PC -D_DARWIN_C_SOURCE -D_XOPEN_SOURCE

SHARED_LINK_FLAGS = -dynamiclib
DLLEXT			= dylib

SHARED_LINK_RPATH = -install_name /opt/staros.xyz/platform/libs/lib$(USER_LIB).$(DLLEXT)


include $(SRC_ROOT)/make/makeinclude/platform_common.mak

