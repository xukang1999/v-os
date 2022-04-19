#
# Copyright (C) 2016-2022 南京北星极网络科技有限公司
#

DEFFLAGS += -DCAW_LINUX -DCAW_UNIX -DVOS_LINUX -DOS_LINUX_PC -D_GNU_SOURCE

SHARED_LINK_FLAGS = -shared
DLLEXT			= so
include $(SRC_ROOT)/make/makeinclude/platform_common.mak

