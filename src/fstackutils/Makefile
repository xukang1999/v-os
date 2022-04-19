include $(USER_DEPTH)/make/makeinclude/mk_pre.mak
USER_LIB = fstackutils

USER_SRCS_CPP = \
src/CFSAcceptorBase.cpp \
src/CFSAcceptorTcp.cpp \
src/CFSAcceptorThreadProxy.cpp \
src/CFSAcceptorUdp.cpp \
src/CFSConnectorThreadProxy.cpp \
src/CFSConnectorWrapper.cpp \
src/CFSEventQueueBase.cpp\
src/CFSManagerImp.cpp \
src/CFSStreamBase.cpp\
src/CFSPipe.cpp\
src/CFSReactorBase.cpp\
src/CFSReactorEpoll.cpp\
src/CFSReactorNotifyPipe.cpp\
src/CFSSocket.cpp \
src/CFSTimerWrapperID.cpp\
src/CFSTransportBase.cpp \
src/CFSThreadReactor.cpp\
src/CFSTransportTcp.cpp \
src/CFSTransportThreadProxy.cpp \
src/CFSTransportUdp.cpp\
fsipstack/CFSIPStack.cpp\
fsipstack/CFSIPStackLib.cpp
USER_INCLDIRS = \
-I$(FSTACK_INCLUDE)\
-I./\
-I./include\
-I./src\
-I./fsipstack\
-I./include/fstackutils

USER_LINK_DLLS=-L$(FSTACK_LIB_PATH)
USER_LINK_DLLS+=-L$(DPDK_LIB_PATH)
#USER_INCLDIRS +=-I$(FSTACK_INCLUDE)
USER_DEFFLAGS =
USER_LINK_DLLS+=
USER_LINK_LIBS = 
USER_LINK_FLAGS= -L$(USER_DEPTH)/target
USER_OBJ_DIR=$(TARGET_PATH)/src/fstackutils
include $(USER_DEPTH)/make/makeinclude/mk_post.mak


