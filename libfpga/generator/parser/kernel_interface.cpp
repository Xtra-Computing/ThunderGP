
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>


#include "parser.h"
#include "parser_debug.h"

using namespace std;

typedef struct
{
    std::string    name;
    arg_instance_t kernel_arg;
} kernel_arg_instance_t;



static std::vector<kernel_arg_instance_t> kernel_arg_list;

static kernel_arg_instance_t * get_kernel_arg(std::string arg)
{
    for (int i = 0; i < kernel_arg_list.size(); i ++)
    {
        if (arg == kernel_arg_list[i].name)
        {
            return &kernel_arg_list[i];
        }
    }
    return NULL;
}

int register_kernel_arg(arg_instance_t item)
{
    if (get_kernel_arg(item.name))
    {
        DEBUG_PRINTF("    error: %s redefine!\n", item.name.c_str());
        return -1;
    }
    kernel_arg_instance_t new_arg;
    new_arg.name  = item.name;
    new_arg.kernel_arg = item;
    DEBUG_PRINTF("    registered %s \n", item.name.c_str());
    DEBUG_PRINTF("    object: %s \n", item.object.c_str());
    kernel_arg_list.push_back(new_arg);
    return 0;
}
static int get_of_number(void)
{
    if (kernel_arg_list.size() > 0)
        return OUTPUT_ATTR_MULTI;

    else
        return OUTPUT_ATTR_SINGLE;
}
static int output_to_file(std::ofstream * of, int ln, int fileid, int gn)
{
    for (int i = 0; i < kernel_arg_list.size(); i ++)
    {
        if (kernel_arg_list[i].kernel_arg.ln == ln )
        {
            return 1;
        }
        else if (kernel_arg_list[i].kernel_arg.ln == ln + 1 )
        {
            std::string object = kernel_arg_list[i].kernel_arg.object;
            while (true)
            {
                bool ret = replace(object, "#%d#", to_string(fileid + 1));
                if (ret == false)
                {
                    break;
                }
            }
            ( *of) << object << std::endl;
            return 1;
        }
    }
    return 0;
}

output_method_t kernel_output_method =
{
    .of_number = get_of_number,
    .output    = output_to_file,
};


static parser_item_t local_parser[] =
{
    {
        .id      = PRAGMA_ID_CU_DUPLICATE,
        .keyword = "MSLR_FUNCTION",
        .func    = register_kernel_arg,
    },
};

reg_parser_item_t kernel_interface_parser =
{
    .pointer = local_parser,
    .size  = ARRAY_SIZE(local_parser),
};
