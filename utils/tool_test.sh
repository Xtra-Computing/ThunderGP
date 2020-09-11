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
            'rmat-24-16.txt' \
            'bio-mouse-gene.edges' \
            'web-Google.mtx'\
            'wiki-Talk.txt'\
            'amazon-2008.mtx' \
            'web-hudong.edges' \
            'web-baidu-baike.edges' \
            'wiki-topcats.mtx' \
            'soc-flickr-und.edges' \
            'pokec-relationships.txt' \
            'LiveJournal1.txt' \
            'wikipedia-20070206.mtx' \
            'ca-hollywood-2009.mtx' \
            'graph500-scale23-ef16_adj.edges' \
            'soc-twitter-2010.mtx' \
            'graph500-scale24-ef16_adj.edges' \
            'graph500-scale25-ef16_adj.edges' \
)

#make app=$2 exe 

for dataset  in "${DATASET[@]}"
do
echo "/graph_data/$dataset"

./host_graph_fpga $1 /graph_data/$dataset > ./${log_path}/$2_$dataset.log

done


