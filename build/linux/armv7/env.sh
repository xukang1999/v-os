#!/bin/sh
#
# Copyright (C) 2013-2015, Nanjing StarOS Technology Co., Ltd
#
CURRENT=`pwd`
export ROOTDIR=$CURRENT/../../..
export ACE_ROOT=$ROOTDIR
export ACE_LIB_PATH=$ACE_ROOT/libs
export ACE_INCLUDE=$ACE_ROOT/include
export CC=gcc
export CXX=g++
