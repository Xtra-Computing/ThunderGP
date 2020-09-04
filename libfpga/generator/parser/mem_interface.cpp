
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
    unsigned int   mask;
    std::string    name;
    arg_instance_t input;
    arg_instance_t attr;
    std::vector<arg_instance_t> instance;
} mem_arg_instance_t;


const int dependence_map[] =
{
    PRAGMA_ID_MEM_ARG,
    PRAGMA_ID_MEM_ATTR,
    PRAGMA_ID_MEM_INSTANCE,
};

static std::vector<mem_arg_instance_t> mem_list;



static mem_arg_instance_t * get_mem_arg(std::string arg)
{
    for (int i = 0; i < mem_list.size(); i ++)
    {
        if (arg == mem_list[i].name)
        {
            return &mem_list[i];
        }
    }
    return NULL;
}

int find_in_dependency(int id)
{
    for (int i = 0; i < ARRAY_SIZE(dependence_map); i++)
    {
        if (id == dependence_map[i])
        {
            return i;
        }
    }
    return -1;
}

unsigned int get_dependency_mask(int id)
{
    int i = 0;
    for (int i = 0; i < ARRAY_SIZE(dependence_map); i++)
    {
        if (id == dependence_map[i])
        {
            unsigned int mask = (1 << i) - 1;
            return mask;
        }
    }
    return 0;

}

int register_mem_arg(arg_instance_t item)
{
    if (get_mem_arg(item.name))
    {
        DEBUG_PRINTF("    error: %s redefine!\n", item.name.c_str());
        return -1;
    }
    mem_arg_instance_t new_arg;
    new_arg.name  = item.name;
    new_arg.input = item;
    new_arg.mask = 0;
    int dep_bit = find_in_dependency(item.id);
    if (dep_bit < 0)
    {
        DEBUG_PRINTF("    error: %s is invalid!\n", item.name.c_str());
        return -1;
    }
    new_arg.mask |= (1 << dep_bit);
    DEBUG_PRINTF("    registered %s \n", item.name.c_str());
    DEBUG_PRINTF("    object: %s \n", item.object.c_str());
    mem_list.push_back(new_arg);
    return 0;
}


int register_mem_attr(arg_instance_t item)
{
    mem_arg_instance_t * p_registered = get_mem_arg(item.name);
    if (p_registered != NULL)
    {
        if (p_registered->mask != get_dependency_mask(item.id))
        {
            DEBUG_PRINTF("    error: dependency mismatch %x of %s\n",
                         p_registered->mask,
                         item.name.c_str());
            return -1;
        }
        p_registered->attr = item;
        int dep_bit = find_in_dependency(item.id);
        if (dep_bit < 0)
        {
            DEBUG_PRINTF("    error: %s is invalid!\n", item.name.c_str());
            return -1;
        }
        p_registered->mask |= (1 << dep_bit);
        p_registered->attr = item;
        DEBUG_PRINTF("    %s attr registered \n", item.name.c_str())
        DEBUG_PRINTF("    object: %s \n", item.object.c_str());
        return 0;
    }
    else
    {
        DEBUG_PRINTF("    error: %s not exist!\n", item.name.c_str());
        return -1;
    }
    return 0;
}



int register_mem_instance(arg_instance_t item)
{
    mem_arg_instance_t * p_registered = get_mem_arg(item.name);
    if (p_registered != NULL)
    {
        if ((p_registered->mask & get_dependency_mask(item.id))
                != get_dependency_mask(item.id))
        {
            DEBUG_PRINTF("    error: dependency mismatch %x of %s instance\n",
                         p_registered->mask,
                         item.name.c_str());
            return -1;
        }
        int dep_bit = find_in_dependency(item.id);
        if (dep_bit < 0)
        {
            DEBUG_PRINTF("    error: %s is invalid!\n", item.name.c_str());
            return -1;
        }
        p_registered->mask |= (1 << dep_bit);
        DEBUG_PRINTF("    %s instance registered \n", item.name.c_str());
        p_registered->instance.push_back(item);
        DEBUG_PRINTF("    %s total registered %d \n", item.name.c_str(), p_registered->instance.size());
        DEBUG_PRINTF("    object: %s \n", item.object.c_str());
        return 0;
    }
    else
    {
        DEBUG_PRINTF("    error: %s not exist!\n", item.name.c_str());
        return -1;
    }
    return 0;
}

static int get_of_number(void)
{
    return OUTPUT_ATTR_SINGLE;
}

static int output_to_file(std::ofstream * of, int ln, int fileid, int gn)
{
    for (int i = 0; i < mem_list.size(); i ++)
    {
        /* MSLR_INTERFACE_ARG */
        if (mem_list[i].input.ln == ln )
        {
            return 1;
        }
        else if (mem_list[i].input.ln == ln + 1 )
        {
            for (int j = 0; j < gn ; j ++)
            {
                std::string object = mem_list[i].input.object;
                while (true)
                {
                    bool ret = replace(object, "#%d#", to_string(j));
                    if (ret == false)
                    {
                        break;
                    }
                }
                ( *of) << object << std::endl;
            }
            return 1;
        }
        /* MSLR_INTERFACE_ATTR */
        if (mem_list[i].attr.ln == ln )
        {
            return 1;
        }
        else if (mem_list[i].attr.ln == ln + 1 )
        {
#if 0
            std::string object ("#pragma HLS INTERFACE s_axilite port=#%d# bundle=control");
            while (true)
            {
                bool ret = replace(object, "#%d#", mem_list[i].name);
                if (ret == false)
                {
                    break;
                }
            }
            ( *of) << object << std::endl;
#endif
            return 1;
        }
        /* MSLR_INTERFACE_INSTANCE */
        for (int k = 0; k < mem_list[i].instance.size(); k++)
        {
            if (mem_list[i].instance[k].ln == ln )
            {
                return 1;
            }
            else if (mem_list[i].instance[k].ln == ln + 1 )
            {
                for (int j = 0; j < gn ; j ++)
                {
                    std::string object = mem_list[i].instance[k].object;
                    while (true)
                    {
                        bool ret = replace(object, "#%d#", to_string(j));
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
    }
    return 0;
}

output_method_t mem_output_method =
{
    .of_number = get_of_number,
    .output    = output_to_file,
};


static parser_item_t local_parser[] =
{
    {
        .id      = PRAGMA_ID_MEM_ARG,
        .keyword = "MSLR_INTERFACE_ARG",
        .func    = register_mem_arg,
    },
    {
        .id      = PRAGMA_ID_MEM_ATTR,
        .keyword = "MSLR_INTERFACE_ATTR",
        .func    = register_mem_attr,
    },
    {
        .id      = PRAGMA_ID_MEM_INSTANCE,
        .keyword = "MSLR_INTERFACE_INSTANCE",
        .func    = register_mem_instance,
    }
};



reg_parser_item_t mem_interface_parser =
{
    .pointer = local_parser,
    .size  = ARRAY_SIZE(local_parser),
};
