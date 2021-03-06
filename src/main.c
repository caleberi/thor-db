#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "./utils/constants.h"
#include "input_buffer.h"

int main(int argc, char **argv)
{
    Table *table = new_table();
    if (argc < 2) {
        printf("Must supply a database filename.\n");
        exit(EXIT_FAILURE);
    }
    char* filename = argv[1];
    Table* table = db_open(filename);
    InputBuffer *input_buffer = new_input_buffer();
    while (true)
    {
        print_prompt();
        read_input(input_buffer);

        // verify if the input is a command
        if (input_buffer->buffer[0] == '.')
        {
            switch (do_meta_command(input_buffer,table))
            {
            case META_COMMAND_SUCCESS:
                continue;
            case META_COMMAND_UNRECOGNIZED_COMMAND:
                printf("Unrecognized command '%s'\n", input_buffer->buffer);
                continue;
            }
        }
        Statement statement;
        // check for correct and supported SQL statement
        switch (prepare_statement(input_buffer, &statement))
        {
        case PREPARE_SUCCESS:
            break;
        case PREPARE_SYNTAX_ERROR:
            printf("Syntax error. Could not parse statement\n");
            continue;
        case PREPARE_UNRECOGNIZED_STATEMENT:
            printf("Unrecongized keyword at  start of '%s'.\n", input_buffer->buffer);
            continue;
        }
        // printf("****** DEBUGGING ******\n");
        switch (execute_statement(&statement, table))
        {
        case EXECUTE_SUCCESS:
            printf("Execute success\n");
            break;
        case EXECUTE_DUPLICATE_KEY:
            printf("Error: Duplicate key.\n");
            break; 
        case EXECUTE_TABLE_FULL:
            printf("Table is full\n");
            break;
        }
        printf("Executed statement :> '%s' \n", input_buffer->buffer);
    }

    return 0;
}
