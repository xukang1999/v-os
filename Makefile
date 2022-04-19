export USER_DEPTH:=$(shell pwd)
include $(USER_DEPTH)/make/makeinclude/mk_pre.mak
USER_SUB_DIRS= src
include $(USER_DEPTH)/make/makeinclude/mk_post.mak



