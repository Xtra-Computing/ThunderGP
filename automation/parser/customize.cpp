#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "parser.h"
#include "parser_debug.h"

#include "customize_str.h"


#define ARG_ITEM(N)   {                 \
        .id      = N,                   \
        .keyword = #N,                  \
        .func    = register_##N,        \
    },

#define MARKER_DEFINE(N) int  N##_ln;   \
    int N##_flag;                       \
    arg_instance_t N##_marker;

#define MARKER_FUNCTION(N,var)                                  \
int register_##N(arg_instance_t item)                           \
{                                                               \
    if (marker.var##_flag == FLAG_RESET)                        \
    {                                                           \
        marker.var##_flag    = FLAG_SET;                        \
        marker.var##_marker  = item;                            \
        marker.var##_ln      = item.ln;                         \
        return 0;                                               \
    }                                                           \
    DEBUG_PRINTF("redef apply %s marker by %s",#var,#N);        \
    return -1;                                                  \
}

#define MARKER_OUTPUT(var,code)                                 \
if (marker.var##_flag == FLAG_SET)                              \
{                                                               \
    if ((ln + 1) == marker.var##_ln )                           \
    {                                                           \
        for (auto args : def_arg_list)                          \
        {                                                       \
            code                                                \
        }                                                       \
        return 1;                                               \
    }                                                           \
}

using namespace std;

typedef enum { VAR_SCALAR = 1, VAR_INPUT_ARRAY, VAR_DUPLEX_ARRAY } var;

typedef struct
{
    std::string    name;
    arg_instance_t input;
    var            var_type;
    int            mem_chn;
} def_arg_instance_t;

typedef struct
{
    MARKER_DEFINE(function)
    MARKER_DEFINE(stream_attr)
    MARKER_DEFINE(scalar_attr)
    MARKER_DEFINE(write)
    MARKER_DEFINE(read_from_stream)
    MARKER_DEFINE(cov)
    MARKER_DEFINE(cov_write)
    MARKER_DEFINE(write_to_stream)
    MARKER_DEFINE(cal)
    MARKER_DEFINE(makefile)
    MARKER_DEFINE(dump)
    MARKER_DEFINE(dump_scalar)
    MARKER_DEFINE(cl)

} arg_mark_t;

typedef struct
{
    std::string type;

    int code_start_ln;
    arg_instance_t start;
    int code_start_flag;

    int code_end_ln;
    arg_instance_t end;
    int code_end_flag;
} code_region_t;

static std::vector<def_arg_instance_t> def_arg_list;
static arg_mark_t marker;
static code_region_t region;

MARKER_FUNCTION(USER_APPLY_ARG, function)
MARKER_FUNCTION(USER_APPLY_STREAM_ATTR, stream_attr)
MARKER_FUNCTION(USER_APPLY_SCALAR_ATTR, scalar_attr)
MARKER_FUNCTION(USER_APPLY_WRITE, write)
MARKER_FUNCTION(USER_APPLY_READ_FROM_STREAM, read_from_stream)
MARKER_FUNCTION(USER_APPLY_COV_FOR_CAL, cov)
MARKER_FUNCTION(USER_APPLY_COV_FOR_WRITE, cov_write)
MARKER_FUNCTION(USER_APPLY_WRITE_TO_STREAM, write_to_stream)
MARKER_FUNCTION(USER_APPLY_CAL, cal)
MARKER_FUNCTION(MAKEFILE_USER_APPLY, makefile)

MARKER_FUNCTION(DUMP_MEM_ATTR, dump);
MARKER_FUNCTION(DUMP_MEM_SCALAR, dump_scalar)

MARKER_FUNCTION(USER_APPLY_CL_KERNEL, cl);


static int gmem_counter = 6;
static int mem_id_offset = 0;
static int output_to_file(std::ofstream * of, int ln, int fileid, int gn)
{
    if (marker.cal_flag == FLAG_SET)
    {
        if ((ln + 1) == marker.cal_ln )
        {
            DEBUG_PRINTF("    enter: %d %d %d\n", ln, fileid, gn);
            if ((region.code_start_flag == FLAG_RESET)
                    || (region.code_end_flag == FLAG_RESET))
            {
                DEBUG_PRINTF("    error: missing code region marker start %d end %d!\n",
                             region.code_start_flag, region.code_end_flag);
                exit(-1);
            }
            if (region.start.file_name != region.end.file_name)
            {
                DEBUG_PRINTF("    error: mismatch file name %s end %s!\n",
                             region.start.file_name.c_str(), region.end.file_name.c_str());
                exit(-1);
            }
            if (region.start.ln > region.end.ln)
            {
                DEBUG_PRINTF("    error: start & end in wrong order!\n");
                exit(-1);
            }
            std::ifstream fhandle(region.start.file_name.c_str());
            if (!fhandle.is_open()) {
                DEBUG_PRINTF("error: can not open %s \n", region.start.file_name.c_str());
                exit(EXIT_FAILURE);
            }
            std::string line;
            int line_number  = 0;
            while (std::getline(fhandle, line)) {
                if ((line_number > region.start.ln ) && line_number < (region.end.ln - 1))
                {
                    ( *of) << line << std::endl;
                }
                line_number ++;
            }
            fhandle.close();
            return 1;
        }
    }

    MARKER_OUTPUT(write_to_stream,
    {
        DEBUG_PRINTF("   write_to_stream enter: %d \n", ln);
        if (args.var_type == VAR_DUPLEX_ARRAY)
        {
            std::string name = args.input.name;
            std::string object("write_to_stream(new%sStream, new%s_u512);");
            replace_all(object, "%s", name);
            ( *of) << object << std::endl;
        }
    })
    MARKER_OUTPUT(cov_write,
    {
        if (args.var_type == VAR_DUPLEX_ARRAY)
        {
            std::string name = args.input.name;
            std::string object(COV_STR_WRITE);
            replace_all(object, "%s", name);
            ( *of) << object << std::endl;
        }
    })
    MARKER_OUTPUT(cov,
    {
        if ((args.var_type == VAR_DUPLEX_ARRAY)
        || ((args.var_type == VAR_INPUT_ARRAY)))
        {
            std::string name = args.input.name;
            std::string object(COV_STR);
            replace_all(object, "%s", name);
            ( *of) << object << std::endl;
        }
    })
    MARKER_OUTPUT(read_from_stream,
    {
        if ((args.var_type == VAR_DUPLEX_ARRAY)
        || ((args.var_type == VAR_INPUT_ARRAY)))
        {
            std::string name = args.input.name;
            std::string object(READ_STR);
            replace_all(object, "%s", name);
            ( *of) << object << std::endl;
        }
        if (args.var_type == VAR_DUPLEX_ARRAY)
        {
            std::string name = args.input.name;
            ( *of) << "burst_raw " << "new" << name << "_u512;" << std::endl;
        }
    })
    MARKER_OUTPUT(function,
    {
        std::string name = args.input.name;

        if (args.var_type == VAR_DUPLEX_ARRAY)
        {
            ( *of) << "uint16    *" << name << "," << std::endl;
            ( *of) << "uint16    *" << "new" << name << "," << std::endl;
        }
        if (args.var_type == VAR_INPUT_ARRAY)
        {
            ( *of) << "uint16    *" << name << "," << std::endl;
        }
        if (args.var_type == VAR_SCALAR)
        {
            ( *of) << region.type << "     " << name << "," << std::endl;
        }
    })
    MARKER_OUTPUT(stream_attr,
    {

        if ((args.var_type == VAR_DUPLEX_ARRAY)
        || ((args.var_type == VAR_INPUT_ARRAY)))
        {
            std::string name = args.input.name;
            std::string object(STREAM_ATTR_STR);
            replace_all(object, "%s", name);
            replace_all(object,  "%d", to_string(gmem_counter));
            ( *of) << object << std::endl;

            if ((args.var_type == VAR_DUPLEX_ARRAY))
            {
                std::string object(STREAM_DUPLEX_OUTPUT_ATTR_STR);
                replace_all(object, "%s",  "new" + name);
                replace_all(object,  "%d", to_string(gmem_counter));
                ( *of) << object << std::endl;
            }
            gmem_counter ++;
        }
    })
    MARKER_OUTPUT(scalar_attr,
    {

        if (args.var_type == VAR_SCALAR)
        {
            std::string name = args.input.name;
            std::string object(SCALAR_ATTR_STR);
            replace_all(object, "%s",  name);
            ( *of) << object << std::endl;
        }
    })
    MARKER_OUTPUT(write,
    {
        if (args.var_type == VAR_DUPLEX_ARRAY)
        {
            std::string name = args.input.name;
            std::string object(WRITE_STR);
            replace_all(object, "%s",  "new" + name);

            ( *of) << object << std::endl;
        }
    })
    MARKER_OUTPUT(dump_scalar,
    {
        if (args.var_type == VAR_SCALAR)
        {
            ( *of) << region.type  << " " << args.input.name << ";" << std::endl;
        }
    })
    MARKER_OUTPUT(dump,
    {
        if ((args.var_type == VAR_DUPLEX_ARRAY)
        || ((args.var_type == VAR_INPUT_ARRAY)))
        {
            mem_id_offset ++;
            std::ostringstream ss1;
            ss1  << " MEM_ID_"
            << args.input.name
            << " "
            << "(MEM_ID_CUSTOM_BASE +"
            << to_string(mem_id_offset)
            << ")"
            << std::endl;
            std::string def_mem_id = ss1.str();
            std::transform(def_mem_id.begin(), def_mem_id.end(), def_mem_id.begin(), ::toupper);

            std::ostringstream ss2;
            ss2  << " MEM_ATTR_"
            << args.input.name
            << " "
            << "("
            << "ATTR_PL_DDR"
            << to_string(args.mem_chn)
            << ")"
            << std::endl;
            std::string def_mem_attr = ss2.str();
            std::transform(def_mem_attr.begin(), def_mem_attr.end(), def_mem_attr.begin(), ::toupper);

            ( *of) << "#define " << def_mem_id << std::endl;
            ( *of) << "#define " << def_mem_attr << std::endl;
        }
        if (args.var_type == VAR_DUPLEX_ARRAY)
        {
            mem_id_offset ++;
            std::ostringstream ss1;
            ss1  << " MEM_ID_"
            << "new"
            << args.input.name
            << " "
            << "(MEM_ID_CUSTOM_BASE +"
            << to_string(mem_id_offset)
            << ")"
            << std::endl;
            std::string def_mem_id = ss1.str();
            std::transform(def_mem_id.begin(), def_mem_id.end(), def_mem_id.begin(), ::toupper);

            std::ostringstream ss2;
            ss2  << " MEM_ATTR_"
            << "new"
            << args.input.name
            << " "
            << "("
            << "ATTR_PL_DDR"
            << to_string(args.mem_chn)
            << ")"
            << std::endl;
            std::string def_mem_attr = ss2.str();
            std::transform(def_mem_attr.begin(), def_mem_attr.end(), def_mem_attr.begin(), ::toupper);

            ( *of) << "#define " << def_mem_id << std::endl;
            ( *of) << "#define " << def_mem_attr << std::endl;
        }


    })
    MARKER_OUTPUT(cl,
    {
        if ((args.var_type == VAR_DUPLEX_ARRAY)
        || ((args.var_type == VAR_INPUT_ARRAY)))
        {
            std::ostringstream ss;
            ss  << " MEM_ID_" << args.input.name;
            std::string mem_id_macro = ss.str();
            std::transform(mem_id_macro.begin(), mem_id_macro.end(), mem_id_macro.begin(), ::toupper);
            ( *of) << "clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer("
            << mem_id_macro
            << "));" << std::endl;
            //he_set_dirty(MEM_ID_RESULT_REG);
        }
        if (args.var_type == VAR_DUPLEX_ARRAY)
        {
            std::ostringstream ss;
            ss  << " MEM_ID_" << "new" << args.input.name;
            std::string mem_id_macro = ss.str();
            std::transform(mem_id_macro.begin(), mem_id_macro.end(), mem_id_macro.begin(), ::toupper);
            ( *of) << "clSetKernelArg(applyHandler->kernel, argvi++, sizeof(cl_mem), get_cl_mem_pointer("
            << mem_id_macro
            << "));" << std::endl;
            ( *of) << "he_set_dirty("
            << mem_id_macro
            << ");" << std::endl;
        }
        if (args.var_type == VAR_SCALAR)
        {
            ( *of) << "clSetKernelArg(applyHandler->kernel, argvi++, 4,    &(global." <<  args.input.name << "));" << std::endl;
        }
    })
    MARKER_OUTPUT(makefile,
    {
        if ((args.var_type == VAR_DUPLEX_ARRAY)
        || ((args.var_type == VAR_INPUT_ARRAY)))
        {
            std::string name = args.input.name;
            std::string object(MAKFILE_STR);
            replace_all(object, "%s", name);
            replace_all(object, "%d", to_string(args.mem_chn));

            ( *of) << object << std::endl;
        }
        if (args.var_type == VAR_DUPLEX_ARRAY)
        {
            std::string name = args.input.name;
            std::string object(MAKFILE_STR);
            replace_all(object, "%s", "new" + name);
            replace_all(object, "%d", to_string(args.mem_chn));
            ( *of) << object << std::endl;
        }
    })
    return 0;
}

static def_arg_instance_t * get_def_arg(std::string arg)
{
    for (int i = 0; i < def_arg_list.size(); i ++)
    {
        if (arg == def_arg_list[i].name)
        {
            return &def_arg_list[i];
        }
    }
    return NULL;
}

int register_base_type(arg_instance_t item)
{
    region.type = item.name;
    DEBUG_PRINTF("    [BASE] %s\n", region.type.c_str());
    return 0;
}

int register_USER_APPLY_CODE_START(arg_instance_t item)
{
    if (region.code_start_flag == FLAG_RESET)
    {
        region.code_start_flag = FLAG_SET;
        region.code_start_ln = item.ln;
        region.start = item;
        DEBUG_PRINTF("    [START] registered @%d \n", item.ln);
        DEBUG_PRINTF("    [START] file: %s \n", item.file_name.c_str());
        return 0;
    }
    return -1;
}


int register_USER_APPLY_CODE_END(arg_instance_t item)
{
    if (region.code_end_flag == FLAG_RESET)
    {
        region.code_end_flag = FLAG_SET;
        region.code_end_ln = item.ln;
        region.end = item;
        DEBUG_PRINTF("    [END] registered @%d \n", item.ln);
        DEBUG_PRINTF("    [END] file: %s \n", item.file_name.c_str());
        return 0;
    }
    return -1;
}


int register_def_scalar(arg_instance_t item)
{
    if (get_def_arg(item.name))
    {
        DEBUG_PRINTF("    error: %s redefine!\n", item.name.c_str());
        return -1;
    }
    def_arg_instance_t new_arg;
    new_arg.name  = item.name;
    new_arg.input = item;
    new_arg.var_type = VAR_SCALAR;
    DEBUG_PRINTF("    registered %s \n", item.name.c_str());
    DEBUG_PRINTF("    object: %s \n", item.object.c_str());
    def_arg_list.push_back(new_arg);
    return 0;
}
static int memory_chn = 0;

int register_def_input_only_array(arg_instance_t item)
{
    if (get_def_arg(item.name))
    {
        DEBUG_PRINTF("    error: %s redefine!\n", item.name.c_str());
        return -1;
    }
    def_arg_instance_t new_arg;
    new_arg.name  = item.name;
    new_arg.input = item;
    new_arg.mem_chn = memory_chn % 4;
    memory_chn ++;
    new_arg.var_type = VAR_INPUT_ARRAY;
    DEBUG_PRINTF("    registered input-only %s \n", item.name.c_str());
    DEBUG_PRINTF("    object: %s \n", item.object.c_str());
    def_arg_list.push_back(new_arg);
    return 0;
}

int register_def_dou_array(arg_instance_t item)
{
    if (get_def_arg(item.name))
    {
        DEBUG_PRINTF("    error: %s redefine!\n", item.name.c_str());
        return -1;
    }
    def_arg_instance_t new_arg;
    new_arg.name  = item.name;
    new_arg.input = item;
    new_arg.mem_chn = memory_chn % 4;
    memory_chn ++;
    new_arg.var_type = VAR_DUPLEX_ARRAY;
    DEBUG_PRINTF("    registered duplex %s \n", item.name.c_str());
    DEBUG_PRINTF("    object: %s \n", item.object.c_str());
    def_arg_list.push_back(new_arg);
    return 0;
}


static int get_of_number(void)
{
    return OUTPUT_ATTR_SINGLE;
}

output_method_t customize_output_method =
{
    .of_number = get_of_number,
    .output    = output_to_file,
};


static parser_item_t local_parser[] =
{
    {
        .id      = DEF_SCALAR,
        .keyword = "DEF_SCALAR",
        .func    = register_def_scalar,
    },
    {
        .id      = DEF_ARRAY,
        .keyword = "DEF_ARRAY",
        .func    = register_def_dou_array,
    },
    {
        .id      = DEF_INPUT_ONLY_ARRAY,
        .keyword = "DEF_INPUT_ONLY_ARRAY",
        .func    = register_def_input_only_array,
    },
    {
        .id      = APPLY_BASE_TYPE,
        .keyword = "APPLY_BASE_TYPE",
        .func    = register_base_type,
    },

    ARG_ITEM(USER_APPLY_ARG)
    ARG_ITEM(USER_APPLY_STREAM_ATTR)
    ARG_ITEM(USER_APPLY_SCALAR_ATTR)
    ARG_ITEM(USER_APPLY_READ_FROM_STREAM)
    ARG_ITEM(USER_APPLY_COV_FOR_CAL)
    ARG_ITEM(USER_APPLY_CAL)
    ARG_ITEM(USER_APPLY_COV_FOR_WRITE)
    ARG_ITEM(USER_APPLY_WRITE_TO_STREAM)
    ARG_ITEM(USER_APPLY_WRITE)
    ARG_ITEM(USER_APPLY_CODE_START)
    ARG_ITEM(USER_APPLY_CODE_END)
    ARG_ITEM(MAKEFILE_USER_APPLY)
    ARG_ITEM(DUMP_MEM_ATTR)
    ARG_ITEM(DUMP_MEM_SCALAR)
    ARG_ITEM(USER_APPLY_CL_KERNEL)
};


reg_parser_item_t customize_interface_parser =
{
    .pointer = local_parser,
    .size  = ARRAY_SIZE(local_parser),
};
