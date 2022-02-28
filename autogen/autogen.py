#!/usr/bin/python3
import os
import sys

# modify here if you want to change the distribution of kernels 
# apply, gs0, gs1, ..., gsn
distribution = {1: [0, 0],
                2: [0, 1, 2],
                3: [0, 0, 1, 2],
                4: [0, 0, 1, 1, 2],
                5: [0, 0, 1, 1, 2, 2],
                6: [0, 0, 0, 1, 1, 2, 2],
                7: [0, 0, 0, 1, 1, 1, 2, 2],
                8: [0, 0, 0, 1, 1, 1, 2, 2, 2],
                9: [0, 0, 0, 0, 1, 1, 1, 2, 2, 2]
                }

def gen_gs_top(acc_template, acc_top, kernel_num):
    with open(acc_template, "r", encoding="utf-8") as f1, open(acc_top, "w", encoding="utf-8") as f2:
        for line in f1:
            f2.write(line)
    

def gen_gs_makefile(acc_template, acc_top, kernel_num):
    THUNDERGP_KERNEL_NUM  = "%THUNDERGP_KERNEL_NUM%"
    THUNDERGP_KERNEL_NAME = "%THUNDERGP_KERNEL_NAME%"
    THUNDERGP_GS_KERNEL_EDGEARRAY_ID  = "%THUNDERGP_GS_KERNEL_EDGEARRAY_ID%"
    THUNDERGP_GS_KERNEL_VERTEXPROP_ID = "%THUNDERGP_GS_KERNEL_VERTEXPROP_ID%"
    THUNDERGP_GS_KERNEL_EDGEPROP_ID   = "%THUNDERGP_GS_KERNEL_EDGEPROP_ID%"
    THUNDERGP_GS_KERNEL_SLR = "%THUNDERGP_GS_KERNEL_SLR%"

    flag = 0
    lines = []

    with open(acc_template, "r", encoding="utf-8") as f1, open(acc_top, "w", encoding="utf-8") as f2:
        for line in f1:
            if THUNDERGP_KERNEL_NAME in line or flag:
                flag = 1
                lines.append(line)
            else:
                line = line.replace(THUNDERGP_KERNEL_NUM, str(kernel_num))
                f2.write(line)

        for idx in range(kernel_num):
            for line in lines:
                print(line)
                tmp_line = line.replace(THUNDERGP_KERNEL_NAME, str(idx+1))
                tmp_line = tmp_line.replace(THUNDERGP_GS_KERNEL_EDGEARRAY_ID, str(2*idx))
                tmp_line = tmp_line.replace(THUNDERGP_GS_KERNEL_VERTEXPROP_ID, str(2*idx+1))
                tmp_line = tmp_line.replace(THUNDERGP_GS_KERNEL_EDGEPROP_ID, str(idx+2*kernel_num))
                tmp_line = tmp_line.replace(THUNDERGP_GS_KERNEL_SLR, str(distribution[kernel_num][idx+1]))
                f2.write(tmp_line)
            f2.write("\n")



def gen_apply_top(acc_template, acc_top, kernel_num):
    THUNDERGP_VERTEXPROP_ID = "%THUNDERGP_VERTEXPROP_ID%"
    THUNDERGP_GMEM_ID = "%THUNDERGP_GMEM_ID%"

    with open(acc_template, "r", encoding="utf-8") as f1, open(acc_top, "w", encoding="utf-8") as f2:
        for line in f1:
            if THUNDERGP_VERTEXPROP_ID in line:
                for idx in range(kernel_num):
                    tmp_line = line.replace(THUNDERGP_VERTEXPROP_ID, str(idx))
                    tmp_line = tmp_line.replace(THUNDERGP_GMEM_ID, str(2*idx+1))
                    f2.write(tmp_line)
            else:
                f2.write(line)




def gen_apply_makefile(acc_template, acc_top, kernel_num):
    THUNDERGP_VERTEXPROP_ID = "%THUNDERGP_VERTEXPROP_ID%"
    THUNDERGP_APPLY_PORT_ID = "%THUNDERGP_APPLY_PORT_ID%"

    with open(acc_template, "r", encoding="utf-8") as f1, open(acc_top, "w", encoding="utf-8") as f2:
        for line in f1:
            if THUNDERGP_VERTEXPROP_ID in line:
                for idx in range(kernel_num):
                    tmp_line = line.replace(THUNDERGP_VERTEXPROP_ID, str(idx))
                    tmp_line = tmp_line.replace(THUNDERGP_APPLY_PORT_ID, str(2*idx+1))
                    f2.write(tmp_line)
            else:
                f2.write(line)




def main():
    if len(sys.argv) < 2:
        raise Exception("Kernel num missing!")

    if len(sys.argv) < 3:
        raise Exception("HAVE_APPLY missing!")

    kernels = int(sys.argv[1])

    if (kernels < 0 or kernels > 8) and sys.argv[2] == 'true':
        raise Exception("Kernel num should greater than or equal to 1 and less than 9!")

    if (kernels < 0 or kernels > 9) and sys.argv[2] == 'false':
        raise Exception("Kernel num should greater than or equal to 1 and less than or equal to 9!")
    
    print("enter main")
    gen_gs_top("./libfpga/common_template/scatter_gather_top.cpp", "./acc_top/scatter_gather_top.cpp", kernels)
    gen_gs_makefile("./libfpga/common_template/gs_kernel.mk", "./acc_top/gs_kernel.mk", kernels)
    gen_apply_top("./libfpga/common_template/apply_top.cpp", "./acc_top/apply_top.cpp", kernels)
    gen_apply_makefile("./libfpga/common_template/apply_kernel.mk", "./acc_top/apply_kernel.mk", kernels)

if __name__ == "__main__":
    main()
