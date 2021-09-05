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
    u_int32_t file_length;
    void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct
{
    uint32_t num_rows;
    Pager* pager;
} Table;

typedef struct {
    Table* table;
    uint32_t row_num;
    // Indicates a position one past the last element
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

void pager_flush(Pager* , uint32_t , uint32_t );

#endif //CONSTANTS_H_