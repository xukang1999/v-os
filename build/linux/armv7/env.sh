#!/bin/sh
#
# Copyright (C) 2013-2015, Nanjing StarOS Technology Co., Ltd
#
CURRENT=`pwd`
export ROOTDIR=$CURRENT/../../..
export _MACHINE=
export _CPU=arm64
export _CPU_TYPE=AARCH64


export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export LD=aarch64-linux-gnu-ld
export AR=aarch64-linux-gnu-ar
export RANLIB=aarch64-linux-gnu-ranlib
export NM=aarch64-linux-gnu-nm
export STRIP=aarch64-linux-gnu-strip
export OBJDUMP=aarch64-linux-gnu-objdump
export AS=aarch64-linux-gnu-as
