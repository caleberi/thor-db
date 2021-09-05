#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include "../input_buffer.h"
#include <stdint.h>
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
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef enum
{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum
{
    PREPARE_SUCCESS,
    PREPARE_NEGATIVE_ID,
    PREPARE_STRING_TOO_LONG,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT

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

typedef struct
{
    uint32_t num_rows;
    void *pages[TABLE_MAX_PAGES];
} Table;

Table *new_table();
void free_table(Table *);
ExecuteResult execute_statement(Statement *, Table *);
ExecuteResult execute_insert(Statement *, Table *);
ExecuteResult execute_select(Statement *, Table *);
PrepareResult prepare_statement(InputBuffer *, Statement *);
PrepareResult prepare_insert(InputBuffer *, Statement *);
MetaCommandResult do_meta_command(InputBuffer *);
void print_row(Row *row);
void serialize_row(Row *, void *);
void deserialize_row(void *, Row *);
void *row_slot(Table *, uint32_t row_num);

#endif //CONSTANTS_H_