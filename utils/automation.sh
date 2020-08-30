#!/bin/bash
make -f ./libfpga/generator/auto_gen_parameters.mk auto_para
make -f ./libfpga/generator/auto_gen_code.mk code_gen
make -f ./libfpga/generator/auto_gen_makefile.mk  makefile_gen 
