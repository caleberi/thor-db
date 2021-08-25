#ifndef INPUT_BUFFER_H_
#define INPUT_BUFFER_H_

#include <stdlib.h>

typedef struct
{
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;

} InputBuffer;

InputBuffer *new_input_buffer();

void print_prompt();
void close_input_buffer(InputBuffer *);
void read_input(InputBuffer *);
#endif // INPUT_BUFFER_H_