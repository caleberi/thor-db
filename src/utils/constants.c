#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include "constants.h"
#include "btree.h"


void print_row(Row *row)
{
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

void print_constants() {
  printf("ROW_SIZE: %d\n", ROW_SIZE);
  printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
  printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
  printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
  printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
  printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}

void print_leaf_node(void* node) {
  uint32_t num_cells = *leaf_node_num_cells(node);
  printf("leaf (size %d)\n", num_cells);
  for (uint32_t i = 0; i < num_cells; i++) {
    uint32_t key = *leaf_node_key(node, i);
    printf("  - %d : %d\n", i, key);
  }
}


Table *db_open(const char* filename)
{
    Pager* pager = pager_open(filename);
    uint32_t num_rows = pager->file_length/ROW_SIZE;
    Table * table = malloc(sizeof(Table));
    table->pager  = pager;
    table->root_page_num = 0;
    if(pager->num_pages==0){
        // New database file .Initialize page 0 as leaf node
        void* root_node = get_page(pager,0);
        initialize_leaf_node(root_node);
    }
    return table;
}

void leaf_node_insert(Cursor* cursor,uint32_t key, Row* value){
    void* node = get_page(cursor->table->pager,cursor->page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);
    if(num_cells>=LEAF_NODE_MAX_CELLS){
        printf("Need to implement splitting a leaf Node");
        exit(EXIT_FAILURE);
    }
    if(cursor->cell_num < num_cells){
        for(uint32_t i=num_cells; i>cursor->cell_num;--i){
            memcpy(leaf_node_cell(node,i),leaf_node_cell(node,i-1),LEAF_NODE_CELL_SIZE);
        }
    }
    *(leaf_node_num_cells(node))+=1;
    *(leaf_node_key(node, cursor->cell_num)) = key;
    serialize_row(value, leaf_node_value(node, cursor->cell_num));
}


Pager* pager_open(const char* filename){
    int fd = open(filename,O_RDWR|O_CREAT|S_IWUSR|S_IRUSR);
    if (fd==-1){
        printf(" Unable to open file with name %s \n",filename);
        exit(EXIT_FAILURE);
    }
    off_t file_length = lseek(fd,0,SEEK_END);
    Pager* pager =  malloc(sizeof(Pager));
    pager->file_descriptor = fd;
    pager->file_length = file_length;
    pager->num_pages =  (file_length/PAGE_SIZE);
    if (file_length%PAGE_SIZE != 0){
        printf("Db file is not a whole number of pages. Corrupt file.\n");
        exit(EXIT_FAILURE);
    }
    for (uint32_t i =0 ;i<TABLE_MAX_PAGES;i++)
        pager->pages[i]=NULL;
    return pager;
}

void* get_page(Pager* pager,uint32_t page_num){
    if(page_num> TABLE_MAX_PAGES){
        printf("Tried to fetch a page number that is out of bound %d > %d \n",page_num,TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }
    if (pager->pages[page_num]==NULL){
        // Cache miss. Allocate memory and load from file.
        void* page = malloc(sizeof(PAGE_SIZE));
        uint32_t num_pages = pager->file_length/PAGE_SIZE;
        if (pager->file_length % PAGE_SIZE) {num_pages += 1;}
        if (page_num <= num_pages) {
            lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
            ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
            if (bytes_read == -1) {
                printf("Error reading file: %d\n", errno);
               exit(EXIT_FAILURE);
                }
        }
        if (page_num >= pager->num_pages) {
            pager->num_pages = page_num + 1;
        }
    }   
    return pager->pages[page_num]; 
}

void pager_flush(Pager* pager, uint32_t page_num) {
  if (pager->pages[page_num] == NULL) {
    printf("Tried to flush null page\n");
    exit(EXIT_FAILURE);
  }

  off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);

  if (offset == -1) {
    printf("Error seeking: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  ssize_t bytes_written =
      write(pager->file_descriptor, pager->pages[page_num],PAGE_SIZE);

  if (bytes_written == -1) {
    printf("Error writing: %d\n", errno);
    exit(EXIT_FAILURE);
  }
}

void db_close(Table* table){
    Pager* pager = table->pager;
    for (uint32_t i = 0; i < pager->num_pages; i++) {
        if (pager->pages[i] == NULL) {
            continue;
        }
        pager_flush(pager, i);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }
    
    int result = close(pager->file_descriptor);
    if (result == -1) {
        printf("Error closing db file.\n");
        exit(EXIT_FAILURE);
    }
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        void* page = pager->pages[i];
        if (page) {
            free(page);
            pager->pages[i] = NULL;
       }
    }
    free(pager);
    free(table);
}

void free_table(Table *table)
{
    for (uint32_t i = 0; table->pager->pages[i]; i++)
    {
        free(table->pager->pages[i]);
    }
    free(table);
}

void serialize_row(Row *source, void *destination)
{
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void *source, Row *destination)
{
    memcpy(&(destination->id), source + ID_SIZE, ID_OFFSET);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}



void* cursor_value(Cursor* cursor){
    uint32_t page_num = cursor->page_num;
    void* page = get_page(cursor->table->pager, page_num);
    return leaf_node_value(page, cursor->cell_num);
}

void advance_cursor(Cursor* cursor){
    uint32_t page_num = cursor->page_num;
    void* node = get_page(cursor->table->pager, page_num);
    cursor->cell_num += 1;
    if (cursor->cell_num >= (*leaf_node_num_cells(node))) {
        cursor->end_of_table = true;
    }
}

MetaCommandResult do_meta_command(InputBuffer *input_buffer,Table* table)
{
    if (strcmp(input_buffer->buffer, TERMINATE_CMD) == 0)
    {
        close_input_buffer(input_buffer);
        db_close(table);
        exit(EXIT_SUCCESS);
    } else if (strcmp(input_buffer->buffer, ".btree") == 0) {
        printf("Tree:\n");
        print_leaf_node(get_page(table->pager, 0));
        return META_COMMAND_SUCCESS;
    }else if (strcmp(input_buffer->buffer, ".constants") == 0) {
       printf("Constants:\n");
       print_constants();
       return META_COMMAND_SUCCESS;
   }
    return META_COMMAND_UNRECOGNIZED_COMMAND;
}

PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement)
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0)
    {
        statement->type = STATEMENT_INSERT;
        int args_count = sscanf(input_buffer->buffer, "insert %d %s %s", &statement->row_to_insert.id,
                                statement->row_to_insert.username, statement->row_to_insert.email);
        if (args_count < 3)
        {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }
    if (strcmp(input_buffer->buffer, "select") == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult execute_insert(Statement *statement, Table *table)
{
    void* node = get_page(table->pager, table->root_page_num);
    uint32_t num_cells = (*leaf_node_num_cells(node));
    if ((*leaf_node_num_cells(node) >= LEAF_NODE_MAX_CELLS)) {
        return EXECUTE_TABLE_FULL;
    }
    Row *row_to_insert = &(statement->row_to_insert);
    uint32_t key_to_insert = row_to_insert->id;
    Cursor* cursor = table_find(table,key_to_insert);
    if(cursor->cell_num < num_cells){
        uint32_t key_at_index = *(leaf_node_key(node,cursor->cell_num));
        if(key_at_index==key_to_insert){
            return EXECUTE_DUPLICATE_KEY;
        }
    }
    free(cursor);
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement *statement, Table *table)
{
    Cursor* cursor = table_start(table);
    Row row;
    while (!(cursor->end_of_table)) {
        deserialize_row(cursor_value(cursor), &row);
        print_row(&row);
        cursor_advance(cursor);
    }
    free(cursor);
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement *statement, Table *table)
{
    switch (statement->type)
    {
    case STATEMENT_INSERT:
        return execute_insert(statement, table);
    case STATEMENT_SELECT:
        return execute_select(statement, table);
    }
}


Cursor* table_start(Table* table){
    Cursor* cursor =  malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->page_num = table->root_page_num;
    void* root_node = get_page(table->pager,table->root_page_num);
    uint32_t num_cells = *leaf_node_num_cells(root_node);
    cursor->end_of_table = (num_cells==0);
    return cursor;
}

Cursor* table_find(Table* table,uint32_t key){
    //Return the position of the given key.
    // the key is not present, 
    // return the positionwhere it should be inserted
    uint32_t root_page_num =  table->root_page_num;
    void* root_node = get_page(table->pager,root_page_num);
    if (get_node_type(root_node)==NODE_LEAF){
        return leaf_node_find(table, root_page_num, key);
    }else{
        printf("Need to implement searching internal nodes");
        exit(EXIT_FAILURE);
    }
}

Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key)
{
  void* node = get_page(table->pager, page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);

  Cursor* cursor = malloc(sizeof(Cursor));
  cursor->table = table;
  cursor->page_num = page_num;

  // Binary search
  uint32_t min_index = 0;
  uint32_t one_past_max_index = num_cells;
  while (one_past_max_index != min_index) {
    uint32_t index = ( one_past_max_index+min_index ) / 2; // ??  overflow
    uint32_t key_at_index = *leaf_node_key(node, index);
    if (key == key_at_index) {
        // set the cursor to the current position before returning it 
        cursor->cell_num = index;
        return cursor;
    }
    // follow the normal binary search algorithm based
    // on where the condition fall to whether mid-1 of mid + 1
    if (key < key_at_index) { 
        one_past_max_index = index;
    } else {
        min_index = index + 1;
    }
  }

  cursor->cell_num = min_index;
  return cursor;
}

NodeType get_node_type(void* node){
    uint32_t value =  (*(uint8_t*)node + NODE_TYPE_OFFSET);
    return (NodeType)value;
}

void set_node_type(void* node,NodeType type){
    uint8_t value = type;
    (*(uint8_t*)(node+NODE_TYPE_OFFSET)) = value;
}