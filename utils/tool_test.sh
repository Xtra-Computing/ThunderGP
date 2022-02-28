#!/bin/bash

tmp_string=`date +%Y%m%d%T`
date_str=${tmp_string//:}
log_path=./test_log_$2_${date_str}

mkdir -p ${log_path}


if [ $# -lt 2 ];
  then
    echo "[FAILD] missing config for start test"
    echo "eg. ------>"
    echo "./tool_test.sh xx.xclbin cc  "
    exit -1
fi


# $1 xclbin
# $2 app

DATASET=(   'rmat-19-32.txt' \
            'rmat-21-32.txt' \
            #'rmat-24-16.txt' \
            'wiki-Talk.txt'\
            'web-Google.mtx'\
            'amazon-2008.mtx' \
            'bio-mouse-gene.edges' \
            'web-hudong.edges' \
            'soc-flickr-und.edges' \
            'web-baidu-baike.edges' \
            'wiki-topcats.mtx' \
            'pokec-relationships.txt' \
            'wikipedia-20070206.mtx' \
            'ca-hollywood-2009.mtx' \
            'LiveJournal1.txt' \
            #'graph500-scale23-ef16_adj.edges' \
            #'soc-twitter-2010.mtx' \
            #'graph500-scale24-ef16_adj.edges' \
            #'graph500-scale25-ef16_adj.edges' \
)

#make app=$2 exe 

for dataset  in "${DATASET[@]}"
do
echo "$dataset"

./host_graph_fpga_$2 $1 /home/xinyuc/graph_dataset/$dataset > ./${log_path}/$2_$dataset.log

cat ./${log_path}/$2_$dataset.log | egrep "edge num|e2e"

done


