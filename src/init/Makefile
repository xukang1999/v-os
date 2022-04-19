
include $(USER_DEPTH)/make/makeinclude/mk_pre.mak

USER_BIN = init

USER_SRCS_CPP = \
main.cpp

USER_LINK_FLAGS= -L$(USER_DEPTH)/target -L/opt/staros.xyz/dipc/dlls  -L/opt/staros.xyz/ace/libs
USER_DEFFLAGS =-fno-builtin -nostdlib -I../ports/include -I../freebsd/include\
-I/opt/staros.xyz/dipc/include\
-I/opt/staros.xyz/ace/include

USER_LINK_DLLS =\
-lfreebsdport\
-lfreebsd\
-lipstack\
-lipstacktp\
-lstarhttp2\
-lstarsctp\
-lscrypto\
-lsslace\
-lsslptcp\
-lpseudotcp\
-lsslutils\
-lstarhttps\
-lstarssh2\
-lpkixssh\
-lutilsex\
-lsoapparser\
-lstarnetconf\
-lusrsctp\
-lnghttp2\
-lstarbase\
-lfipc\
-lsqlitedb\
-lshttp -lzlib -lwface\
-lsjs -lexpat \
-ljson\
-lffi\
-lvos -llibsec -lrtc -lACE -lopenssl -ldl -lutil -pthread

USER_OBJ_DIR=$(TARGET_PATH)/src/dipc

include $(USER_DEPTH)/make/makeinclude/mk_post.mak



