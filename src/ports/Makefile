
include $(USER_DEPTH)/make/makeinclude/mk_pre.mak

USER_LIB = freebsdport

USER_SRCS_CPP = \
src/port_atomic.cpp\
src/port_init.cpp\
src/port_sx_lock.cpp\
src/port_stdout.cpp\
src/port_rw_lock.cpp\
src/port_thread.cpp\
src/port_time.cpp\
src/port_critical.cpp\
src/port_memory.cpp\
src/port_cpu.cpp\
src/port_rand.cpp\
src/port_mmap.cpp\
src/port_process.cpp\
src/port_errno.cpp
USER_INCLDIRS =\
-I/opt/staros.xyz/dipc/include\
-I$(USER_DEPTH)/src/freebsd/include\
-I$(USER_DEPTH)/src/ports/include\
-I$(USER_DEPTH)/src/ports/src
USER_LINK_FLAGS= -L$(USER_DEPTH)/target\
-L/opt/staros.xyz/dipc/dlls
USER_DEFFLAGS =\
-DSO_SHARED_LIBRARY\
-DSO_COMPILE_LIBRARY\
-fno-builtin -nostdlib\
-DWEBRTC_POSIX

USER_LINK_DLLS =\
-lfreebsd\
-lfshmipc\
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

USER_OBJ_DIR=$(TARGET_PATH)/src/ports

include $(USER_DEPTH)/make/makeinclude/mk_post.mak



