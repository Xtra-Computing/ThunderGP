include ThunderGP.mk

PARA_GEN_CFLAGS := -I libfpga/generator/devices
PARA_GEN_CFLAGS += -DDEVICE_HEADER="$(DEVICES).h"
PARA_GEN_CFLAGS += -I libgraph/



.PHONY: auto_para para_gen tmp_para/para.mk
auto_para: tmp_para/para.mk

VAR_TRUE=true
APP = $(app)
APPCONFIG = ./application/$(APP)
include $(APPCONFIG)/build.mk

PARA_GEN_CFLAGS += -DTARGET_BANDWIDTH=$(TARGET_BANDWIDTH)

para_gen:
	rm -rf para_gen
	rm -rf tmp_para
	mkdir -p tmp_para
	g++  -g -static-libstdc++  $(PARA_GEN_CFLAGS)   ./libfpga/generator/para_gen.cpp -o para_gen

tmp_para/para.mk: para_gen
	./para_gen
	rm -rf code_gen
