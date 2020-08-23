#ifndef __PARSER_H__
#define __PARSER_H__

#define MAX_CHAR_LENGTH         (256)

#define PRAGMA_ID_CU_DUPLICATE  (0)
#define PRAGMA_ID_MEM_ARG       (1)
#define PRAGMA_ID_MEM_ATTR      (2)
#define PRAGMA_ID_MEM_INSTANCE  (3)


#define OUTPUT_ATTR_SINGLE      (1)
#define OUTPUT_ATTR_MULTI       (2)

typedef struct
{
    std::string orginal;
    std::string object;
    std::string name;
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

typedef int (*get_of_attr)(void);
typedef int (*output_line)(std::ofstream * of, int ln,int fileid, int gn);

typedef struct
{
    get_of_attr  of_number;
    output_line  output;
} output_method_t;


bool replace(std::string& str, const std::string& from, const std::string& to);


#endif /*__PARSER_H__ */
