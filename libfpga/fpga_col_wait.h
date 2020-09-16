
for (int k = 0; k < EDGE_NUM; k ++) {
#pragma HLS UNROLL
    wait_mask_and_l1[k] = wait_cache_mask[k][0] | wait_cache_mask[k][1];
}
for (int k = 0; k < EDGE_NUM / 2; k ++) {
#pragma HLS UNROLL
    wait_mask_and_l2[k] = wait_mask_and_l1[2 * k] | wait_mask_and_l1[2 * k + 1] ;
}
for (int k = 0; k < EDGE_NUM / 4; k ++) {
#pragma HLS UNROLL
    wait_mask_and_l3[k] = wait_mask_and_l2[2 * k] | wait_mask_and_l2[2 * k + 1] ;
}
wait_cache_flag = wait_mask_and_l3[0] | wait_mask_and_l3[1];


for (int unit_cycle = 0; unit_cycle < 2; unit_cycle ++) {
#pragma HLS UNROLL
    for (int k = 0; k < EDGE_NUM; k ++) {
#pragma HLS UNROLL
        if (curr_caching_value[k][unit_cycle] >=  vertex_index[k][unit_cycle])
        {
            max_passed[k][unit_cycle] = 0xffffffff;
        }
        else
        {
            max_passed[k][unit_cycle] = vertex_index[k][unit_cycle];
        }
    }
}
for (int k = 0; k < EDGE_NUM; k ++) {
#pragma HLS UNROLL
    max_passed_l1[k] = MIN(max_passed[k][0] , max_passed[k][1]);
}
for (int k = 0; k < EDGE_NUM / 2; k ++) {
#pragma HLS UNROLL
    max_passed_l2[k] = MIN(max_passed_l1[2 * k] , max_passed_l1[2 * k + 1]);
}
for (int k = 0; k < EDGE_NUM / 4; k ++) {
#pragma HLS UNROLL
    max_passed_l3[k] = MIN(max_passed_l2[2 * k] , max_passed_l2[2 * k + 1]) ;
}
min_processing_value = MIN(max_passed_l3[0] , max_passed_l3[1]);