#
# Copyright (C) 2016-2020 南京北星极网络科技有限公司
#
ifeq (macos,$(os))
include $(SRC_ROOT)/make/makeinclude/platform_macos.mak
else
include $(SRC_ROOT)/make/makeinclude/platform_linux.mak
endif