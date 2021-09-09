#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include "../input_buffer.h"
#include <stdint.h>
#include <stdbool.h>
#ifndef TERMINATE_CMD
#define TERMINATE_CMD ".quit"
#endif
#ifndef COLUMN_USERNAME_SIZE
#define COLUMN_USERNAME_SIZE 32
#endif
#ifndef COLUMN_EMAIL_SIZE
#define COLUMN_EMAIL_SIZE 255
#endif
#ifndef TABLE_MAX_PAGES
#define TABLE_MAX_PAGES 100
#endif
#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

typedef struct
{
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;


typedef enum
{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum
{
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR
} PrepareResult;

typedef enum
{
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

typedef enum
{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
} ExecuteResult;

typedef struct
{
    Row row_to_insert;
    StatementType type;
} Statement;


typedef struct {
    int file_descriptor;
    uint32_t file_length;
    uint32_t  num_pages;
    void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct
{
    uint32_t root_page_num;
    Pager* pager;

} Table;

typedef struct {
    Table* table;
    uint32_t page_num;
    uint32_t cell_num;
    bool end_of_table; 
} Cursor;

Table *db_open(const char* );
Pager* pager_open(const char* );
Cursor* table_start(Table* );
Cursor* table_end(Table* );
void* get_page(Pager* ,uint32_t );
void free_table(Table *);
ExecuteResult execute_statement(Statement *, Table *);
ExecuteResult execute_insert(Statement *, Table *);
ExecuteResult execute_select(Statement *, Table *);
PrepareResult prepare_statement(InputBuffer *, Statement *);
MetaCommandResult do_meta_command(InputBuffer *,Table* );
void print_row(Row *row);
void serialize_row(Row *, void *);
void deserialize_row(void *, Row *);
void* cursor_value(Cursor* );
void advance_cursor(Cursor* );
void leaf_node_insert(Cursor* ,uint32_t , Row* );
void pager_flush(Pager* , uint32_t );


const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;


#endif //CONSTANTS_H_