#ifndef __CUSTOMIZE_STR_H__
#define __CUSTOMIZE_STR_H__



#define  STREAM_ATTR_STR ("\n\
#pragma HLS INTERFACE m_axi port=%s offset=slave bundle=gmem%d \n\
#pragma HLS INTERFACE s_axilite port=%s bundle=control\n\
        hls::stream<burst_raw>      %sStream;\n\
#pragma HLS stream variable=%sStream depth=16\n\
        burstReadLite(addrOffset, vertexNum, %s, %sStream);\n\
        ")
#define STREAM_DUPLEX_OUTPUT_ATTR_STR ("\n\
#pragma HLS INTERFACE m_axi port=%s offset=slave bundle=gmem%d \n\
#pragma HLS INTERFACE s_axilite port=%s bundle=control \n\
        hls::stream<burst_raw>      %sStream;\n\
#pragma HLS stream variable=%sStream depth=16\n\
")


#define SCALAR_ATTR_STR ("\n\
#pragma HLS INTERFACE s_axilite port=%s      bundle=control \n\
")

#define WRITE_STR ("\n        \
writeBackLite(vertexNum, %s + (addrOffset >> 4), %sStream);\n\
")
#define READ_STR ("\n\
burst_raw %s_u512;\n\
read_from_stream(%sStream, %s_u512);\n\
")

#define COV_STR ("type_cov %s_tmp; \n\
%s_tmp.ui=%s_u512.range((i + 1) * INT_WIDTH - 1, i * INT_WIDTH );\n\
float %s=%s_tmp.f;")


#define COV_STR_WRITE ("type_cov new%s_tmp;\n\
new%s_tmp.f =new%s;\n\
new%s_u512.range((i + 1) * INT_WIDTH - 1, i * INT_WIDTH ) = new%s_tmp.ui;")

#define MAKFILE_STR ("\n\
BINARY_LINK_OBJS    += --sp  vertexApply_1.%s:DDR[%d]")


#endif /* __CUSTOMIZE_STR_H__ */
