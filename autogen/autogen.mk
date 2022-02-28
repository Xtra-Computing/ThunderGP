include ThunderGP.mk

.PHONY: autogen

autogen:
	rm -rf ./acc_top
	mkdir -p ./acc_top
	touch ./acc_top/scatter_gather_top.cpp
	touch ./acc_top/gs_kernel.mk
	touch ./acc_top/apply_top.cpp
	touch ./acc_top/apply_kernel.mk
	python3 ./autogen/autogen.py $(KERNEL_NUM) $(HAVE_APPLY) 