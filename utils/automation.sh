#!/bin/bash
make -f ./autogen/autogen.mk app=$1 KERNEL_NUM=$2 HAVE_APPLY=$3 autogen
# make -f ./automation/auto_gen_code.mk  app=$1 code_gen
# make -f ./automation/auto_gen_makefile.mk app=$1 makefile_gen