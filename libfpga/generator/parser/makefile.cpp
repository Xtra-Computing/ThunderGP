
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
    arg_instance_t input;
} mk_arg_instance_t;

static std::vector<mk_arg_instance_t> mk_list;

int register_makefile_instance(arg_instance_t item)
{
    mk_arg_instance_t mk_item;
    mk_item.input = item;
    mk_list.push_back(mk_item);
    return 0;
}

static int get_of_number(void)
{
    return OUTPUT_ATTR_SINGLE;
}

static int output_to_file(std::ofstream * of, int ln, int fileid, int gn)
{
    for (int i = 0; i < mk_list.size(); i ++)
    {
        /* MSLR_INTERFACE_ARG */
        if (mk_list[i].input.ln == ln )
        {
            return 1;
        }
        else if (mk_list[i].input.ln == ln + 1 )
        {
            for (int j = 0; j < gn ; j ++)
            {
                std::string object = mk_list[i].input.object;
                while (true)
                {
                    bool ret = replace(object, "$(%d)", to_string(j));
                    if (ret == false)
                    {
                        break;
                    }
                }
                ( *of) << object << std::endl;
            }
            return 1;
        }
    }
    return 0;
}

output_method_t makefile_output_method =
{
    .of_number = get_of_number,
    .output    = output_to_file,
};


static parser_item_t local_parser[] =
{
    {
        .id      = MAKEFILE_MEM_INSTANCE,
        .keyword = "MAKEFILE_MEM_INSTANCE",
        .func    = register_makefile_instance,
    }
};



reg_parser_item_t makefile_interface_parser =
{
    .pointer = local_parser,
    .size  = ARRAY_SIZE(local_parser),
};
