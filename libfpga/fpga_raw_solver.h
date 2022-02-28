#ifndef __FPGA_RAW_SOLVER_H__
#define __FPGA_RAW_SOLVER_H__


#include "graph_fpga.h"

#include "fpga_application.h"


#define LOCAL_BUFFER_SIZE       (2)
/* 2 distance */
void processEdgesReorderStreamScheme1(hls::stream<int2>  &in , hls::stream<int2> &out)
{
#pragma HLS function_instantiate variable=in
    int2 local_buffer[LOCAL_BUFFER_SIZE];
#pragma HLS ARRAY_PARTITION variable=local_buffer dim=0 complete

    for (int i = 0; i < LOCAL_BUFFER_SIZE ; i++)
    {
        local_buffer[i].x = 0;
        local_buffer[i].y = 0;
    }

    int buffered_size = 0;
    while (true)
    {
        int2 tmp_data;
        read_from_stream(in, tmp_data);
        uint_raw dstVidx  = tmp_data.x;

        int i = buffered_size & 0x01;
        if (local_buffer[i].x == dstVidx)
        {
            DEBUG_PRINTF("is %d %d %d\n", int (dstVidx), int (local_buffer[i].y), int (tmp_data.y));
            local_buffer[i].y = PROP_COMPUTE_STAGE2(local_buffer[i].y, tmp_data.y);

        }
        else
        {
            if (i)
            {
                local_buffer[0] = tmp_data;
                write_to_stream(out, local_buffer[1]);
            }
            else
            {
                local_buffer[1] = tmp_data;
                write_to_stream(out, local_buffer[0]);
            }
            buffered_size ++;
        }
        // (ENDFLAG -1) for partition align
        // (ENDFLAG)    for stream end,
        // we cut the write stream here to avoid the flag be added to cache
        if ((dstVidx & ( ENDFLAG - 1 )) == (ENDFLAG - 1))
        {
            break;
        }
    }
    //write_to_stream(out, local_buffer[buffered_size & 0x01]);
    empty_stream(in);

    int2 tmp_data;
    tmp_data.x = ENDFLAG;
    tmp_data.y = ENDFLAG;
    write_to_stream(out, tmp_data);
}


/* 4 distance */
void rawSolver(hls::stream<int2_token>  &in , hls::stream<int2_token> &out)
{
#pragma HLS function_instantiate variable=in
    int2 local_buffer[5];
#pragma HLS ARRAY_PARTITION variable=local_buffer dim=0 complete
    ap_uint<1> vaild_flag[4];
#pragma HLS ARRAY_PARTITION variable=vaild_flag dim=0 complete

    int2 out_data[2];

    uint_raw lastSendBuffered[4];
#pragma HLS ARRAY_PARTITION variable=lastSendBuffered dim=0 complete


    for (int i = 0; i < 5 ; i++)
    {
        local_buffer[i].x = 0;
        local_buffer[i].y = 0;
    }
    for (int i = 0; i < 4 ; i++)
    {
        vaild_flag[i] = 0;
        lastSendBuffered[i] = 0;
    }

    int buffered_size = 0;
    while (true)
    {
#pragma HLS PIPELINE II=2
        int2 tmp_data;
        int2_token in_token;


        ap_uint<3> select_index = 4;
        read_from_stream(in, in_token);
        tmp_data = in_token.data;
        uint_raw dstVidx  = tmp_data.x;

        ap_uint<1> flag[4];
        // (ENDFLAG -1) for partition align
        // (ENDFLAG)    for stream end,
        // we cut the write stream here to avoid the flag be added to cache
        if ((dstVidx & ( ENDFLAG - 1 )) == (ENDFLAG - 1) || (in_token.flag == FLAG_SET))
        {
            break;
        }
        for (int i = 0; i < 4; i++)
        {
#pragma HLS UNROLL
            if ((vaild_flag[i] == 1) && (local_buffer[i].x  ==  dstVidx))
            {
                local_buffer[i].y  = PROP_COMPUTE_STAGE2(local_buffer[i].y, tmp_data.y);
                flag[i] = 1;
            }
            else
            {
                flag[i] = 0;
            }
        }

        if (flag[0] | flag[1] | flag[2] | flag[3])
        {
            //DEBUG_PRINTF("is %d %d %d\n", int (dstVidx), int (local_buffer[i].y), int (tmp_data.y));
        }
        else
        {
            {
#pragma HLS latency min=1 max=1
                if (vaild_flag[0] == 0)
                {
                    local_buffer[0] = tmp_data;
                    vaild_flag[0] = 1;
                }
                else if (vaild_flag[1] == 0)
                {
                    local_buffer[1] = tmp_data;
                    vaild_flag[1] = 1;
                }
                else if (vaild_flag[2] == 0)
                {
                    local_buffer[2] = tmp_data;
                    vaild_flag[2] = 1;
                }
                else if (vaild_flag[3] == 0)
                {
                    local_buffer[3] = tmp_data;
                    vaild_flag[3] = 1;
                }
            }
            unsigned int abs[4];
#pragma HLS ARRAY_PARTITION variable=abs dim=0 complete
            {
#pragma HLS latency min=1 max=1

                for (int i = 0; i < 4; i++)
                {
#pragma HLS UNROLL
                    if (vaild_flag[i] == 0)
                    {
                        abs[i] = 0;
                    }
                    else
                    {
                        if (lastSendBuffered[i] > local_buffer[i].x)
                        {
                            abs[i] = lastSendBuffered[i] - local_buffer[i].x;
                        }
                        else
                        {
                            abs[i] = local_buffer[i].x - lastSendBuffered[i];
                        }
                    }
                }
            }
            int2 send;
            {
#pragma HLS latency min=1 max=1

#define     DST_INTERVEL    (32)
                if (abs[0] >= DST_INTERVEL)
                {
                    select_index = 0;
                    //send = local_buffer[0];
                    vaild_flag [0] = 0;
                }
                else if (abs[1] >= DST_INTERVEL)
                {
                    select_index = 1;
                    //send = local_buffer[1];
                    vaild_flag [1] = 0;
                }
                else if (abs[2] >= DST_INTERVEL)
                {
                    select_index = 2;
                    //send = local_buffer[2];
                    vaild_flag [2] = 0;
                }
                else if (abs[3] >= DST_INTERVEL)
                {
                    select_index = 3;
                    //send = local_buffer[3];
                    vaild_flag [3] = 0;
                }
                else
                {
                    select_index = 4;
                }
                out_data[0] = local_buffer[select_index];
            }
            send = out_data[0];
            if (select_index != 4)
            {
                for (int k = 0; k < 4 ; k++)
                {
#pragma HLS UNROLL
                    lastSendBuffered[k] = send.x;
                }
                int2_token send_token;
                send_token.data = send;
                send_token.flag = FLAG_RESET;
                write_to_stream(out, send_token);
            }
        }

    }
    for (int i = 0; i < 4; i++)
    {
#pragma HLS PIPELINE II=3
        if (vaild_flag[i] == 1)
        {

            int2 send;
            send.x = local_buffer[i].x + DST_INTERVEL;
            send.y = 0;

            int2_token send_token;
            send_token.data = send;
            send_token.flag = FLAG_RESET;
            write_to_stream(out, send_token);
            write_to_stream(out, send_token);
            send_token.data = local_buffer[i];
            write_to_stream(out, send_token);
        }
    }

    //write_to_stream(out, local_buffer[buffered_size & 0x01]);
    empty_stream(in);

    int2_token send_token;
    send_token.flag = FLAG_SET;
    write_to_stream(out, send_token);
}



#endif /* __FPGA_RAW_SOLVER_H__ */
