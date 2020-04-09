#!/bin/sh
xbutil reset -h
xbutil program -p /data/graph_fpga.xclbin

xbutil dmatest

make exe