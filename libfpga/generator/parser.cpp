
#include <string>
#include <cctype>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include <stdarg.h>
#include <vector>


#include "parser.h"
#include "parser_debug.h"
#include "mem_interface.h"
#include "kernel_interface.h"

using namespace std;

parser_item_t local_item[] =
{
    {
        .id      = PRAGMA_ID_CU_DUPLICATE,
        .keyword = "MSLR_FUNCTION",
        .func    = register_kernel_arg,
    },
    {
        .id      = PRAGMA_ID_MEM_ARG,
        .keyword = "INTERFACE_ARG",
        .func    = register_mem_arg,
    },
    {
        .id      = PRAGMA_ID_MEM_ATTR,
        .keyword = "INTERFACE_HLS_ATTR",
        .func    = register_mem_attr,
    },
    {
        .id      = PRAGMA_ID_MEM_INSTANCE,
        .keyword = "INTERFACE_HLS_INSTANCE",
        .func    = register_mem_instance,
    }
};

std::string find_arg(void * context, std::string &line, int ln)
{
    parser_item_t * p_context = (parser_item_t *)context;
    std::string arg = line;
    std::string target(p_context->pragma_string);
    target.erase(std::remove_if(target.begin(), target.end(), ::isspace), target.end());
    size_t pos = arg.find(target);
    if (pos != std::string::npos)
    {
        arg.erase(pos, target.length());
        DEBUG_PRINTF("ARG: %s of %s in line: %d\n", arg.c_str(), p_context->keyword, ln);
    }
    return arg;
}



int identify_pragmas(std::string &current_line, std::string &next_line, int line_number)
{
    std::string stripped_line = current_line;
    stripped_line.erase(
        std::remove_if(stripped_line.begin(), stripped_line.end(), ::isspace),
        stripped_line.end()
    );
    arg_instance_t arg_ins;
    for (int i = 0; i < ARRAY_SIZE(local_item); i++)
    {
        std::string target(local_item[i].pragma_string);
        target.erase(std::remove_if(target.begin(), target.end(), ::isspace), target.end());
        if (stripped_line.find(target) != std::string::npos) {
            arg_ins.name    = find_arg(&local_item[i], stripped_line, line_number);
            arg_ins.orginal = current_line;
            arg_ins.ln      = line_number;
            arg_ins.id      = local_item[i].id;
            arg_ins.object  = next_line;

            if (local_item[i].func != NULL)
            {
                int ret = local_item[i].func(arg_ins);
                if (ret < 0)
                {
                    exit(EXIT_FAILURE);
                }
            }
            return 0;
        }
    }
    return 1;
}

int file_input(const std::string& input)
{
    std::ifstream fhandle(input.c_str());
    if (!fhandle.is_open()) {
        DEBUG_PRINTF("error: can not open %s \n", input.c_str());
        exit(EXIT_FAILURE);
    }

    std::string line;
    std::string last_line;
    int line_number  = 0;
    while (std::getline(fhandle, line)) {
        if (line_number > 0)
        {
            identify_pragmas(last_line, line, (line_number));
        }
        line_number ++;
        last_line  = line;
    }
    fhandle.close();
    return 0;
}


std::vector<output_method_t> output_methods;

int register_output_method(output_method_t method)
{
    output_methods.push_back(method);
    return 0;
}

int parser_init(void)
{
    const char * prefix = "#pragma THUNDERGP";
    for (int i = 0; i < ARRAY_SIZE(local_item); i++)
    {
        size_t n = sprintf(local_item[i].pragma_string, "%s %s", prefix, local_item[i].keyword);
        if (n > MAX_CHAR_LENGTH)
        {
            DEBUG_PRINTF("bug: pragma string overflow\n");
            exit(EXIT_FAILURE);
        }
    }
    register_output_method(kernel_output_method);
    register_output_method(mem_output_method);
    return 0;
}
#define GN  (4)

std::vector<std::ofstream *> output_files;

int file_output(const std::string& input, const std::string& output)
{
    int of_attr = OUTPUT_ATTR_SINGLE;
    for (auto method : output_methods)
    {
        if (method.of_number() > of_attr)
        {
            of_attr = method.of_number();
        }
    }
    int total_file_num;
    int group_num = GN;

    if (of_attr == OUTPUT_ATTR_SINGLE)
        total_file_num = 1;
    else
        total_file_num = group_num;

    for (int i = 0; i < total_file_num; i++)
    {

        std::ostringstream output_file_name;
        output_file_name << output;
        output_file_name << "_";
        output_file_name << i;
        output_file_name << ".cpp";
        std::ofstream * of = new std::ofstream(output_file_name.str().c_str());
        output_files.push_back(of);

    }

    std::ifstream fhandle(input.c_str());
    if (!fhandle.is_open()) {
        DEBUG_PRINTF("error: can not open %s \n", input.c_str());
        exit(EXIT_FAILURE);
    }

    std::string line;
    std::string last_line;
    int line_number  = 0;
    while (std::getline(fhandle, line)) {

        int ret = 0;
        for (int i = 0; i < output_methods.size(); i++)
        {
            for (int j = 0; j < total_file_num; j++)
            {
                if (output_methods[i].output)
                {
                    ret += output_methods[i].output(output_files[j],
                                                    line_number,
                                                    j,
                                                    group_num);
                }
            }
        }

        if (ret == 0)
        {
            for (int i = 0; i < total_file_num; i++)
            {
                (*output_files[i]) << line << std::endl;
            }
        }
        line_number ++;
    }
    fhandle.close();

    for (int i = 0; i < total_file_num; i++)
    {
        (*output_files[i]).flush();
        (*output_files[i]).close();
    }
    return 0;
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

int main(int argc, char **argv) {
    std::string input_code;
    parser_init();
    if (argc > 1)
    {
        input_code = argv[1];
    }
    else
    {
        DEBUG_PRINTF("error: no input \n");
    }
    file_input(input_code);
    if (argc > 2)
    {
        file_output(input_code, argv[2]);
    }
    return 0;
}