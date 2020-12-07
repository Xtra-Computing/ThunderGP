#!/bin/bash
#make -f ./automation/auto_gen_parameters.mk app=$1 auto_para
make -f ./automation/auto_gen_code.mk  app=$1 code_gen
#make -f ./automation/auto_gen_makefile.mk app=$1 makefile_gen
