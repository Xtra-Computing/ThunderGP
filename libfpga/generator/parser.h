#ifndef __PARSER_H__
#define __PARSER_H__

#include "common.h"

#define MAX_CHAR_LENGTH         (256)

#define PRAGMA_ID_CU_DUPLICATE  (0)

#define PRAGMA_ID_MEM_ARG       (1)
#define PRAGMA_ID_MEM_ATTR      (2)
#define PRAGMA_ID_MEM_INSTANCE  (3)

#define USER_APPLY_ARG          (4)
#define USER_APPLY_STREAM_ATTR  (5)
#define USER_APPLY_SCALAR_ATTR  (6)




#define USER_APPLY_WRITE              (10)
#define USER_APPLY_READ_FROM_STREAM   (11)
#define USER_APPLY_COV_FOR_CAL        (12)
#define USER_APPLY_COV_FOR_WRITE      (13)
#define USER_APPLY_WRITE_TO_STREAM    (14)
#define USER_APPLY_CAL                (15)

#define DEF_SCALAR                    (7)
#define DEF_INPUT_ONLY_ARRAY          (8)
#define DEF_ARRAY                     (9)
#define USER_APPLY_CODE_START         (16)
#define USER_APPLY_CODE_END           (17)

#define MAKEFILE_MEM_INSTANCE         (18)
#define MAKEFILE_USER_APPLY           (19)
#define DUMP_MEM_ATTR                 (20)
#define DUMP_MEM_SCALAR               (21)
#define USER_APPLY_CL_KERNEL          (22)
#define APPLY_BASE_TYPE               (23)


#define OUTPUT_ATTR_SINGLE      (1)
#define OUTPUT_ATTR_MULTI       (2)

#define PRAGMA_PERFIX           "#pragma THUNDERGP"


typedef struct
{
    std::string orginal;
    std::string object;
    std::string name;
    std::string file_name;
    int ln;
    int id;
} arg_instance_t;


typedef int (*parser_func)(arg_instance_t item);

typedef struct
{
    const int    id;
    const char   *keyword;

    parser_func  func;
    char         pragma_string[MAX_CHAR_LENGTH];

} parser_item_t;

typedef struct
{
    parser_item_t * pointer;
    int size;
} reg_parser_item_t;

typedef int (*get_of_attr)(void);
typedef int (*output_line)(std::ofstream * of, int ln, int fileid, int gn);

typedef struct
{
    get_of_attr  of_number;
    output_line  output;
} output_method_t;


bool replace(std::string& str, const std::string& from, const std::string& to);

inline int replace_all(std::string &object, std::string sub, std::string target)
{
    while (true)
    {
        bool ret = replace(object, sub, target);
        if (ret == false)
        {
            break;
        }
    }
    return 0;
}


#endif /*__PARSER_H__ */
