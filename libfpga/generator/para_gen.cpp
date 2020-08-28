#include <stdio.h>

#include "device_common.h"

#ifndef DEVICE_HEADER
#error "no board name"
#endif

#include DEVICE_HEADER

int main(int argc, char **argv) {
	printf("%s \n", board_name);
}