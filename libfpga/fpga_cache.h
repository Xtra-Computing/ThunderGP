#ifndef __FPGA_CACHE_H__
#define __FPGA_CACHE_H__


#include "graph_fpga.h"



#define URAM_DEPTH              (4096)
#define URAM_PER_EDGE           (4)
#define CACHE_SIZE              (URAM_DEPTH * URAM_PER_EDGE * 2)

#define CACHE_ADDRESS_MASK      (URAM_DEPTH * 8 - 1)

#define CACHE_UPDATE_BURST      (1<<LOG_CACHEUPDATEBURST)
#define LOG_CACHEUPDATEBURST    (LOG_SCATTER_CACHE_BURST_SIZE)

#define CA_WIDTH                (32)
//(32 + 1 - LOG_CACHEUPDATEBURST - LOG2_SIZE_BY_INT)


typedef  struct
{
    burst_raw data;
    uint_raw addr;
} cache_line;


typedef  struct
{
    uint_raw idx;
    uint_raw size;
    ap_uint<1> flag;
} cache_command;

typedef struct
{
    burst_raw tuples;
    burst_raw score;
    ap_uint<1> flag;
} edgeBlock;


typedef struct {
    ap_uint<CA_WIDTH*SIZE_BY_INT>   data;
    ap_uint<5>                      num;
    ap_uint<1>                      flag;
} address_token;

typedef struct {
    ap_uint<CA_WIDTH*SIZE_BY_INT>   data;
    ap_uint<1>                      flag;
} filtered_token;



template <typename T>
void streamMerge(
    hls::stream<T>              &edge,
    hls::stream<T>              &score,
    hls::stream<edgeBlock>      &edgeBlockStream
)
{
    while (true)
    {
        edgeBlock tempEdgeBlock;
        T         tempEdge;
        T         tempScore;
        read_from_stream(edge , tempEdge);
        read_from_stream(score, tempScore);
        tempEdgeBlock.tuples = tempEdge.data;
        tempEdgeBlock.score = tempScore.data;
        tempEdgeBlock.flag = tempEdge.flag;
        write_to_stream(edgeBlockStream, tempEdgeBlock);
        if (tempEdge.flag == FLAG_SET)
        {
            break;
        }
    }
    /* TODO  control signal improvement*/
    {
        edgeBlock end;
        end.flag = FLAG_SET;
        write_to_stream(edgeBlockStream, end);
    }
    //empty_stream(edge);
    //empty_stream(score);

}


void writeTuples( hls::stream<edge_tuples_t>  &edgeTuplesBuffer, edge_tuples_t (&tuples)[2])
{
#pragma HLS INLINE OFF
    for (int unit_cycle = 0; unit_cycle < 2; unit_cycle ++ )
    {
#pragma HLS UNROLL

        write_to_stream(edgeTuplesBuffer, tuples[unit_cycle]);

    }
}
//uint_raw address = (((uint_raw)(outer_idx) << (LOG_CACHEUPDATEBURST + LOG2_SIZE_BY_INT)) & CACHE_ADDRESS_MASK) >> 3
// the preload size is calculated by
inline uint_raw cacheUpdateByAddr(
    cache_line                      &cache_data,
    uint_uram                       vertexPropCache[EDGE_NUM][URAM_PER_EDGE][URAM_DEPTH])
{
#pragma HLS INLINE
    {
        uint_raw uram_addr = (cache_data.addr & CACHE_ADDRESS_MASK) >> 3;

        for (int j = 0 ; j < 2 ; j ++)
        {
            for (int k = 0; k < EDGE_NUM; k++)
            {
#pragma HLS UNROLL
                vertexPropCache[k][0][uram_addr + j] = cache_data.data.range(63 +  (j << 8) + 64 * 0, 0 + (j << 8) + 64 * 0);
                vertexPropCache[k][1][uram_addr + j] = cache_data.data.range(63 +  (j << 8) + 64 * 1, 0 + (j << 8) + 64 * 1);
                vertexPropCache[k][2][uram_addr + j] = cache_data.data.range(63 +  (j << 8) + 64 * 2, 0 + (j << 8) + 64 * 2);
                vertexPropCache[k][3][uram_addr + j] = cache_data.data.range(63 +  (j << 8) + 64 * 3, 0 + (j << 8) + 64 * 3);
            }
        }
        return cache_data.addr;
    }
}



template <typename T>
void  duplicateStreamForCache(hls::stream<T>       &input,
                              hls::stream<T>       &output1,
                              hls::stream<T>       &output2)
{
#pragma HLS function_instantiate variable=input
    while (true)
    {
#pragma HLS PIPELINE II=1
        T  unit;
        read_from_stream(input, unit);
        write_to_stream(output1, unit);
        //if (unit != lastUnit)
        {
            write_to_stream(output2, unit);
        }
        if (unit.flag == FLAG_SET)
        {
            break;
        }
    }
}

void streamFilter(hls::stream<burst_token>         &mapStream,
                  hls::stream<filtered_token>      &filteredStream)
{
    uint_raw last_index =  ENDFLAG - 1;
    ap_uint<1> update_flag = 0;
    while (true)
    {
        burst_token map;
        filtered_token output;
        read_from_stream(mapStream, map);
        for (int i = 0; i < SIZE_BY_INT - 1; i++)
        {
#pragma HLS UNROLL
            output.data.range((CA_WIDTH - 1) + i * CA_WIDTH, 0 + i * CA_WIDTH)
                = map.data.range(31 + i * 32, 0 + i * 32) >> (LOG_CACHEUPDATEBURST + LOG2_SIZE_BY_INT);
        }

        uint_raw min_index = (output.data.range(31, 0) );
        uint_raw max_index = ((map.data.range(511, 480) + 64) >> (LOG_CACHEUPDATEBURST + LOG2_SIZE_BY_INT));

        output.data.range((CA_WIDTH - 1) + (CA_WIDTH * (SIZE_BY_INT - 1)), 0 + (CA_WIDTH * (SIZE_BY_INT - 1)) )
            = ((map.data.range(511, 480) + 64) >> (LOG_CACHEUPDATEBURST + LOG2_SIZE_BY_INT));


        if ((last_index == (ENDFLAG - 1)) || (min_index > last_index) || (max_index > last_index))
        {
            update_flag = 1;
        }
        else
        {
            update_flag = 0;
        }
        if (map.flag == FLAG_SET)
        {
            break;
        }
        else if (update_flag)
        {

            if ((max_index != last_index) || (map.flag == FLAG_SET))
            {
                last_index = max_index;
                output.flag = map.flag;
                write_to_stream(filteredStream, output);
            }
        }
    }
    {
        filtered_token token;
        token.flag = FLAG_SET;
        write_to_stream(filteredStream, token);
        return;
    }
}

#define INVALID_FLAG (0xffffffff)
void streamRemoveBubble(hls::stream<filtered_token> &in,
                       hls::stream<address_token>  &out)
{
    ap_uint < CA_WIDTH > ori[SIZE_BY_INT];
#pragma HLS ARRAY_PARTITION variable=ori dim=0 complete

    ap_uint < CA_WIDTH > tmp_array[SIZE_BY_INT][SIZE_BY_INT];
#pragma HLS ARRAY_PARTITION variable=tmp_array dim=0 complete

    ap_uint < CA_WIDTH > array[SIZE_BY_INT][SIZE_BY_INT];
#pragma HLS ARRAY_PARTITION variable=array dim=0 complete

    ap_uint<1> mask[SIZE_BY_INT];
#pragma HLS ARRAY_PARTITION variable=mask dim=0 complete

    ap_uint<2> level_1_sum[8];
#pragma HLS ARRAY_PARTITION variable=level_1_sum dim=0 complete

    ap_uint<3> level_2_sum[4];
#pragma HLS ARRAY_PARTITION variable=level_2_sum dim=0 complete

    ap_uint<4> level_3_sum[2];
#pragma HLS ARRAY_PARTITION variable=level_3_sum dim=0 complete


    while (true)
    {
        filtered_token in_data;
        address_token out_data;
        read_from_stream(in, in_data);
        for (int j = 0; j < SIZE_BY_INT; j++)
        {
#pragma HLS UNROLL
            ori[j] = in_data.data.range((CA_WIDTH - 1) + j * CA_WIDTH, 0 + j * CA_WIDTH);
        }

        array[0][0] = ori[0];
        mask[0] = 1;
        for (int j = 1; j < SIZE_BY_INT; j++)
        {
#pragma HLS UNROLL
            if (ori[j - 1] != ori[j])
            {
                array[0][j] = ori[j];
                mask[j] = 1;
            }
            else
            {
                array[0][j] = INVALID_FLAG;
                mask[j] = 0;
            }
        }

        for (int j = 0; j < 8; j++)
        {
#pragma HLS UNROLL
            level_1_sum[j] = mask[2 * j] + mask[2 * j + 1];
        }

        for (int j = 0; j < 4; j++)
        {
#pragma HLS UNROLL
            level_2_sum[j] = level_1_sum[2 * j] + level_1_sum[2 * j + 1];
        }

        for (int j = 0; j < 2; j++)
        {
#pragma HLS UNROLL
            level_3_sum[j] = level_2_sum[2 * j] + level_2_sum[2 * j + 1];
        }

        ap_uint<5> result = level_3_sum[0] + level_3_sum[1];

        for (int i = 1; i < 16; i++)
        {
#pragma HLS PIPELINE
            {
                for (int j = 0; j < SIZE_BY_INT - 1 ; j++)
                {
#pragma HLS UNROLL
                    if (array[i - 1][j] == INVALID_FLAG)
                    {
                        tmp_array[i][j] = array[i - 1][j + 1];
                    }
                    else
                    {
                        tmp_array[i][j] = array[i - 1][j];
                    }

                }
                if (array[i - 1][SIZE_BY_INT - 1] == INVALID_FLAG)
                {
                    tmp_array[i][SIZE_BY_INT - 1] = INVALID_FLAG;
                }
                else
                {
                    tmp_array[i][SIZE_BY_INT - 1] = array[i - 1][SIZE_BY_INT - 1];
                }
            }
            {
                array[i][0] = tmp_array[i][0];
                for (int j = 1; j < SIZE_BY_INT; j++)
                {

#pragma HLS UNROLL
                    if (tmp_array[i][j] == tmp_array[i][j - 1])
                    {
                        array[i][j] = INVALID_FLAG;
                    }
                    else
                    {
                        array[i][j] = tmp_array[i][j];
                    }

                }
            }
//            for (int k = 0; k < SIZE_BY_INT ; k++)
//                printf("%d \n", array[i][k]);
//            printf("----------------------\n");
        }
        for (int j = 0; j < SIZE_BY_INT; j++)
        {
#pragma HLS UNROLL
            out_data.data.range((CA_WIDTH - 1) + j * CA_WIDTH, 0 + j * CA_WIDTH) = array[15][j];
        }
        out_data.num = result;
        out_data.flag = in_data.flag;

        write_to_stream(out, out_data);


        if (in_data.flag == FLAG_SET)
        {
            break;
        }
    }
}

void updateVertexCacheNarrow(uint16                          * input,
                             hls::stream<address_token>      &addressStream,
                             hls::stream<cache_line>         &cacheStream)
{

    burst_raw read_buffer[CACHE_UPDATE_BURST];

    ap_uint < CA_WIDTH > ori[SIZE_BY_INT];
#pragma HLS ARRAY_PARTITION variable=ori dim=0 complete

    unsigned int last_index  = INVALID_FLAG;

    while (true)
    {
        address_token addr;
        read_from_stream(addressStream, addr);
        if (addr.flag == FLAG_SET)
        {
            break;
        }

        for (int j = 0; j < SIZE_BY_INT; j++)
        {
#pragma HLS UNROLL
            ori[j] = addr.data.range((CA_WIDTH - 1) + j * CA_WIDTH, 0 + j * CA_WIDTH);
        }

        //C_PRINTF("updating %d  %d  from %d \n", (int)min_index, (int)max_index, (int)last_index)

        for (uint_raw i = 0; i < addr.num ; i ++)
        {
            if ((last_index == INVALID_FLAG) || (ori[i] > last_index))
            {
                uint_raw outer_idx = (ori[i]) & (((2 * 1024 * 1024 * 1024u) >> LOG_CACHEUPDATEBURST) - 1);
                for (int inner_idx = 0 ; inner_idx < CACHE_UPDATE_BURST; inner_idx ++) {
#pragma HLS PIPELINE II=1
                    read_buffer[inner_idx] = input[((uint_raw)(outer_idx) << LOG_CACHEUPDATEBURST) + inner_idx];
                }
                uint_raw address = ((uint_raw)(outer_idx) << (LOG_CACHEUPDATEBURST + LOG2_SIZE_BY_INT));
                for (int inner_idx = 0 ; inner_idx < CACHE_UPDATE_BURST; inner_idx ++)
                {
                    cache_line  cache_data;
                    cache_data.addr = address + (inner_idx << 4);
                    cache_data.data = read_buffer[inner_idx];
                    write_to_stream(cacheStream, cache_data);
#if 0
                    for (int j = 0 ; j < 4 ; j ++)
                    {
                        for (int k = 0; k < EDGE_NUM; k++)
                        {
#pragma HLS UNROLL
                            vertexPropCache[k][0][address + (inner_idx << 2) + j] = read_buffer[inner_idx].range(63 +  (j << 7), 0 + (j << 7));
                            vertexPropCache[k][1][address + (inner_idx << 2) + j] = read_buffer[inner_idx].range(63 +  (j << 7) + 64, 0 + (j << 7) + 64);
                        }
                    }
#endif
                }
            }
            last_index = ori[i];
        }

    }
    {
        cache_line  end;
        end.addr = (ENDFLAG - 15);
        end.data = 0x0;
        write_to_stream(cacheStream, end);
    }
    clear_stream(addressStream);
}





/* TODO: support narrow burst */
void stream2Command(hls::stream<burst_token>        &mapStream,
                    hls::stream<cache_command>      &cmdStream)
{
    uint_raw last_index = ENDFLAG - 1;
    ap_uint<1> update_flag = 0;
    while (true)
    {
        burst_token map;
        cache_command cmd;
        read_from_stream(mapStream, map);

        uint_raw min_index = (map.data.range(31, 0) >> (LOG_CACHEUPDATEBURST + LOG2_SIZE_BY_INT));
        uint_raw max_index = ((map.data.range(511, 480) + 64) >> (LOG_CACHEUPDATEBURST + LOG2_SIZE_BY_INT));
        if ((last_index == (ENDFLAG - 1)) || (min_index > last_index) || (max_index > last_index))
        {
            update_flag = 1;
        }
        else
        {
            update_flag = 0;
        }
        if (update_flag)
        {
            uint_raw min_bound;
            if ((last_index == (ENDFLAG - 1) ) || (min_index > last_index))
            {
                min_bound = min_index;
            }
            else
            {
                min_bound = last_index + 1;
            }
            cmd.idx  =  min_bound;
            cmd.size =  max_index + 2 - min_bound;
            cmd.flag =  FLAG_RESET;
            write_to_stream(cmdStream, cmd);
            last_index = max_index + 1;
        }
        if (map.flag == FLAG_SET)
        {
            break;
        }
    }
    cache_command cmd;
    cmd.idx = 0;
    cmd.size = 0;
    cmd.flag = FLAG_SET;
    write_to_stream(cmdStream, cmd);
}


#define DELAY_BUFFER       (512)
#define LOG_DELAY_BUFFER   (9)
#define POOL_SIZE          (4096)

void streamDelayScheme1(hls::stream<burst_raw>  &in, hls::stream<burst_raw>   &out)
{
    burst_raw  buffer[POOL_SIZE];
#pragma HLS RESOURCE variable=buffer core=XPM_MEMORY uram
    uint_raw   counter = 0;
    while (true)
    {
        burst_raw in_data;
        burst_raw out_data;

        read_from_stream(in, in_data);
        buffer[counter.range(LOG_DELAY_BUFFER - 1, 0)] = in_data;
        if (in_data.range(31, 0) == ENDFLAG)
        {
            break;
        }

        if (counter >= (DELAY_BUFFER - 1))
        {
            out_data = buffer[(counter.range(LOG_DELAY_BUFFER - 1, 0) + 1) & (DELAY_BUFFER - 1)];
            write_to_stream(out, out_data);
        }
        counter ++;
    }
    for (int i = 0; i < DELAY_BUFFER; i ++)
    {
        burst_raw end_data;
        end_data = buffer[(counter.range(LOG_DELAY_BUFFER - 1, 0) + 1 + i) & (DELAY_BUFFER - 1)];
        write_to_stream(out, end_data);
    }

}


void streamDelayScheme2(hls::stream<burst_token>  &in, hls::stream<burst_token>   &out)
{
    burst_raw  buffer[POOL_SIZE];
#pragma HLS RESOURCE variable=buffer core=XPM_MEMORY uram
#pragma HLS DEPENDENCE variable=buffer inter false
#pragma HLS DEPENDENCE variable=buffer intra false

    uint_raw   gr_counter = 0;
    uint_raw   gw_counter = 0;
    while (true)
    {

        burst_raw out_data;
        ap_uint<1> end_flag;
        uint_raw  r_counter = gr_counter;
        uint_raw  w_counter = gw_counter;
//#pragma HLS DEPENDENCE variable=w_counter intra false
//#pragma HLS DEPENDENCE variable=r_counter intra false

        out_data = buffer[gw_counter & (POOL_SIZE - 1)];

        if ((w_counter + DELAY_BUFFER - 1 < r_counter ))
        {
            if (!out.full())
            {
                burst_token out_token;
                out_token.data = out_data;
                out_token.flag = FLAG_RESET;
                write_to_stream(out, out_token);
                gw_counter ++;
            }
        }

        if ((w_counter + POOL_SIZE - 2) > r_counter)
        {
            if (!in.empty())
            {
                burst_token in_data;
                read_from_stream(in, in_data);
                buffer[r_counter & (POOL_SIZE - 1)] = in_data.data;
                gr_counter ++;
                if (in_data.flag == FLAG_SET)
                {
                    end_flag = 1;
                }
                else {
                    end_flag = 0;
                }
            }
        }
        /* magic */
        if (end_flag && r_counter > 16)
        {
            break;
        }
    }
    for (int i = gw_counter; i < gr_counter; i ++)
    {
        burst_token end_token;
        end_token.data = buffer[i & (POOL_SIZE - 1)];
        if (i == (gr_counter - 1))
        {
            end_token.flag = FLAG_SET;
        } else
        {
            end_token.flag = FLAG_RESET;
        }
        write_to_stream(out, end_token);
    }

}

void updateVertexCache(uint16                          * input,
                       hls::stream<cache_command>      &cmdStream,
                       hls::stream<cache_line>         &cacheStream)
{

    burst_raw read_buffer[CACHE_UPDATE_BURST];
    while (true)
    {
        cache_command cmd;
        read_from_stream(cmdStream, cmd);
        C_PRINTF("update cache index %d with size %d , flag %d \n", (int)cmd.idx, (int)cmd.size, (int)cmd.flag);

        for (uint_raw i = 0; i < cmd.size ; i ++)
        {
            uint_raw outer_idx = (i + cmd.idx) & (((2 * 1024 * 1024 * 1024) >> LOG_CACHEUPDATEBURST) - 1);
            for (int inner_idx = 0 ; inner_idx < CACHE_UPDATE_BURST; inner_idx ++) {
#pragma HLS PIPELINE II=1
                read_buffer[inner_idx] = input[((uint_raw)(outer_idx) << LOG_CACHEUPDATEBURST) + inner_idx];
            }
            uint_raw address = ((uint_raw)(outer_idx) << (LOG_CACHEUPDATEBURST + LOG2_SIZE_BY_INT));
            for (int inner_idx = 0 ; inner_idx < CACHE_UPDATE_BURST; inner_idx ++)
            {
                cache_line  cache_data;
                cache_data.addr = address + (inner_idx << 4);
                cache_data.data = read_buffer[inner_idx];
                write_to_stream(cacheStream, cache_data);
#if 0
                for (int j = 0 ; j < 4 ; j ++)
                {
                    for (int k = 0; k < EDGE_NUM; k++)
                    {
#pragma HLS UNROLL
                        vertexPropCache[k][0][address + (inner_idx << 2) + j] = read_buffer[inner_idx].range(63 +  (j << 7), 0 + (j << 7));
                        vertexPropCache[k][1][address + (inner_idx << 2) + j] = read_buffer[inner_idx].range(63 +  (j << 7) + 64, 0 + (j << 7) + 64);
                    }
                }
#endif
            }
        }
        if (cmd.flag == FLAG_SET)
        {
            break;
        }
    }
    {
        cache_line  end;
        end.addr = (ENDFLAG - 15);
        end.data = 0x0;
        write_to_stream(cacheStream, end);
    }
    clear_stream(cmdStream);
}


void  readEdgesStage(
    hls::stream<edge_tuples_t>      &edgeTuplesBuffer,
    hls::stream<edgeBlock>          &edgeBlockStream,
    hls::stream<cache_line>         &cacheStream,
    uint_uram                       vertexPropCache[EDGE_NUM][URAM_PER_EDGE][URAM_DEPTH]
)
{
#pragma HLS dependence variable=vertexPropCache inter false
#pragma HLS DEPENDENCE variable=vertexPropCache intra false

    C_PRINTF("%s \n", "start readedges");
    ap_uint<1>  break_flag = 0;
    uint_raw caching_value = 0;
    uint_raw processing_value = 0;
    uint_raw min_processing_value = 0;
    cache_line  cache_data[2];
    uint_raw    caching_counter = 0;
    edgeBlock   tmpBlock[2];
    uint_raw    processing_counter = 0;

    while (true)
    {
#pragma HLS PIPELINE II=2
        if (break_flag == 1)
        {
            break;
        }
        if (!cacheStream.empty() && (min_processing_value + CACHE_SIZE - 64) > caching_value)
        {
            read_from_stream(cacheStream, cache_data[0]);
            caching_value = (cache_data[0].addr);
            if (caching_counter > 0)
            {
                cacheUpdateByAddr(cache_data[1], vertexPropCache);
            }
            cache_data[1] = cache_data[0];
            caching_counter ++;
        }
        //else
        if ((!edgeBlockStream.empty() && ((processing_value) < caching_value  ) ) || (processing_value == ENDFLAG))
        {
            read_from_stream(edgeBlockStream, tmpBlock[0]);
            processing_value = tmpBlock[0].score.range(511, 511 - 31);
            min_processing_value = tmpBlock[0].score.range(31, 0);
            if (processing_counter > 0)
            {
#pragma HLS latency min=4 max=10
                edge_tuples_t tuples[2];
readCache: for (int unit_cycle = 0; unit_cycle < 2; unit_cycle ++)
                {
#pragma HLS UNROLL
readCacheInner: for (int k = 0; k < EDGE_NUM; k ++) {
#pragma HLS UNROLL
#define  range_start  (( k ) << INT_WIDTH_SHIFT)

                        tuples[unit_cycle].data[k].x =
                            tmpBlock[1].tuples.range((range_start) + 31 + unit_cycle * 256, range_start + unit_cycle * 256);
                        unsigned int vertex_index =
                            tmpBlock[1].score.range((range_start) + 31 + unit_cycle * 256, range_start + unit_cycle * 256);
                        //tuples[0].data[k].y = get_cached_value(vertex_index, vertexPropCache);

                        unsigned int address = (vertex_index & CACHE_ADDRESS_MASK) >> 3;
                        unsigned int bit =  ((vertex_index & CACHE_ADDRESS_MASK) >> 1) & (URAM_PER_EDGE - 1);

                        uint_uram tmp;
                        {
#pragma HLS latency min=1 max=3
                            tmp = vertexPropCache[k][bit][address];
                        }

                        if (vertex_index & 0x01)
                            tuples[unit_cycle].data[k].y = tmp.range(63, 32);
                        else
                            tuples[unit_cycle].data[k].y = tmp.range(31,  0);
#if CAHCE_FETCH_DEBUG
                        if (tuples[unit_cycle].data[k].y != vertex_index)
                        {
                            C_PRINTF("[FETCH] error %d %d\n", tuples[unit_cycle].data[k].y, vertex_index);
                        }
                        else
                        {
                            C_PRINTF("[FETCH] dump %d %d\n", tuples[unit_cycle].data[k].y, vertex_index);
                        }
#endif
                    }
                    tuples[unit_cycle].flag = (tmpBlock[1].flag);
                }
                writeTuples(edgeTuplesBuffer, tuples);
                break_flag = tmpBlock[1].flag;
            }
            tmpBlock[1] = tmpBlock[0];
            processing_counter ++;
        }
    }

    C_PRINTF("%s\n", "end");
    empty_stream(edgeBlockStream);
    empty_stream(cacheStream);
    C_PRINTF("%s\n", "end2");
    return;

}
void srcPropertyProcess( uint16                             * vertexPushinProp,
                         hls::stream<burst_token>           &edgeBurstStream,
                         hls::stream<burst_token>           &mapStream,
                         hls::stream<edge_tuples_t>         &edgeTuplesBuffer
                       )
{
#pragma HLS DATAFLOW
    uint_uram vertexPropCache[EDGE_NUM][URAM_PER_EDGE][URAM_DEPTH];
#pragma HLS ARRAY_PARTITION variable=vertexPropCache dim=1 complete
#pragma HLS ARRAY_PARTITION variable=vertexPropCache dim=2 complete
#pragma HLS RESOURCE variable=vertexPropCache core=XPM_MEMORY uram

#pragma HLS DEPENDENCE variable=vertexPropCache inter false
#pragma HLS DEPENDENCE variable=vertexPropCache intra false

    hls::stream<cache_line>    cacheUpdateStream;
#pragma HLS stream variable=cacheUpdateStream  depth=512



    hls::stream<burst_token>      delayingMapStream;
#pragma HLS stream variable=delayingMapStream depth=2

    hls::stream<burst_token>      delayedMapStream;
#pragma HLS stream variable=delayedMapStream depth=2

    hls::stream<burst_token>      map4CacheStream;
#pragma HLS stream variable=map4CacheStream depth=2

    hls::stream<edgeBlock>      edgeBlockStream;
#pragma HLS stream variable=edgeBlockStream depth=2

    duplicateStreamForCache(mapStream, delayingMapStream, map4CacheStream);

    streamDelayScheme2(delayingMapStream, delayedMapStream);
// two flag_set
    streamMerge(edgeBurstStream, delayedMapStream, edgeBlockStream);


#if 1

    hls::stream<cache_command>    cmdStream;
#pragma HLS stream variable=cmdStream  depth=512

    stream2Command(map4CacheStream, cmdStream);

    updateVertexCache(vertexPushinProp , cmdStream, cacheUpdateStream);
#else

    hls::stream<filtered_token>      filteredStream;
#pragma HLS stream variable=filteredStream depth=2

    hls::stream<address_token>    addressStream;
#pragma HLS stream variable=addressStream depth=512

    streamFilter(map4CacheStream, filteredStream);

    streamRemoveBubble(filteredStream, addressStream);

    updateVertexCacheNarrow(vertexPushinProp , addressStream, cacheUpdateStream);


#endif

    readEdgesStage(edgeTuplesBuffer, edgeBlockStream, cacheUpdateStream, vertexPropCache);
}

#endif /* __FPGA_CACHE_H__ */

