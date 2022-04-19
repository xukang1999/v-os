#!/bin/sh
#
# Copyright (C) 2013-2015, Nanjing StarOS Technology Co., Ltd
#
CURRENT=`pwd`
export ROOTDIR=$CURRENT/../../..
export ACE_ROOT=$ROOTDIR
export ACE_LIB_PATH=$ACE_ROOT/libs
export ACE_INCLUDE=$ACE_ROOT/include
export CC=/opt/gcc-4.9.3-64-gnu/bin/mips64el-linux-gcc
export CXX=/opt/gcc-4.9.3-64-gnu/bin/mips64el-linux-g++
export LD=/opt/gcc-4.9.3-64-gnu/bin/mips64el-linux-ld
export AR=/opt/gcc-4.9.3-64-gnu/bin/mips64el-linux-ar
export RANLIB=/opt/gcc-4.9.3-64-gnu/bin/mips64el-linux-ranlib
export NM=/opt/gcc-4.9.3-64-gnu/bin/mips64el-linux-nm
export STRIP=/opt/gcc-4.9.3-64-gnu/bin/mips64el-linux-strip
export OBJDUMP=/opt/gcc-4.9.3-64-gnu/bin/mips64el-linux-objdump
export AS=/opt/gcc-4.9.3-64-gnu/bin/mips64el-linux-as

