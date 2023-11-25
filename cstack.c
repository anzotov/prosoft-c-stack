#include "cstack.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#define RESULT_OK 0
#define RESULT_NOT_OK 1
#define INVALID_HANDLER -1
#define INITIAL_SIZE 10

typedef struct node
{
    struct node *prev;
    unsigned int size;
    char data[0];
} node;

typedef struct stack
{
    node *top;
    unsigned int size;
} stack;

typedef struct stack_entry
{
    int valid;
    stack stack;
} stack_entry;

typedef struct stack_entries_table
{
    hstack_t size;
    stack_entry *entries;
} stack_entries_table;

stack_entries_table stacks = {0u, NULL};

int allocate_table(void)
{
    assert(!stacks.entries);
    stacks.entries = calloc(INITIAL_SIZE, sizeof(stack_entry));
    if (stacks.entries)
        stacks.size = INITIAL_SIZE;
    else
    {
        stacks.entries = calloc(1, sizeof(stack_entry));
        if (stacks.entries)
            stacks.size = 1;
        else
            return RESULT_NOT_OK;
    }
    return RESULT_OK;
}

void reallocate_table(void)
{
    assert(stacks.entries);
    hstack_t new_size = stacks.size * 2;
    stack_entry *new_array = calloc(new_size, sizeof(stack_entry));

    if (!new_array)
    {
        new_size = stacks.size + 1;
        new_array = calloc(new_size, sizeof(stack_entry));
    }

    if (new_array)
    {
        memcpy(new_array, stacks.entries, stacks.size * sizeof(stack_entry));
        free(stacks.entries);
        stacks.entries = new_array;
        stacks.size = new_size;
    }
}

void deallocate_table(void)
{
    free(stacks.entries);
    stacks.entries = NULL;
    stacks.size = 0;
}

int is_table_empty(void)
{
    for (hstack_t index = 0; index < stacks.size; ++index)
    {
        if (stacks.entries[index].valid)
            return RESULT_NOT_OK;
    }
    return RESULT_OK;
}

hstack_t get_free_index(void)
{
    hstack_t index = 0;
    for (; index < stacks.size && stacks.entries[index].valid; ++index)
        ;
    return index;
}

hstack_t stack_new(void)
{
    if (!stacks.entries)
        if (allocate_table() != RESULT_OK)
            return INVALID_HANDLER;

    hstack_t index = get_free_index();

    if (index >= stacks.size)
        reallocate_table();

    if (index < stacks.size)
    {
        stacks.entries[index].valid = 1;
        return index;
    }
    return INVALID_HANDLER;
}

void stack_free(const hstack_t hstack)
{
    if (stack_valid_handler(hstack) != RESULT_OK)
        return;

    stack *stack = &stacks.entries[hstack].stack;
    node *cur_node = stack->top;

    stacks.entries[hstack].valid = 0;
    stack->top = NULL;
    stack->size = 0;

    node *prev_node;
    while (cur_node)
    {
        prev_node = cur_node->prev;
        free(cur_node);
        cur_node = prev_node;
    }

    if (is_table_empty() == RESULT_OK)
        deallocate_table();
}

int stack_valid_handler(const hstack_t hstack)
{
    if (hstack >= 0 && hstack < stacks.size && stacks.entries[hstack].valid)
        return RESULT_OK;
    return RESULT_NOT_OK;
}

unsigned int stack_size(const hstack_t hstack)
{
    if (stack_valid_handler(hstack) == RESULT_OK)
        return stacks.entries[hstack].stack.size;
    return 0;
}

void stack_push(const hstack_t hstack, const void *data_in, const unsigned int size)
{
    if (!data_in || !size || stack_valid_handler(hstack) != RESULT_OK)
        return;

    stack *stack = &stacks.entries[hstack].stack;
    if (stack->size == UINT_MAX)
        return;

    node *new_node = malloc(sizeof(node) + size);
    if (!new_node)
        return;

    new_node->prev = stack->top;
    new_node->size = size;
    memcpy(new_node->data, data_in, size);

    stack->top = new_node;
    stack->size += 1;
}

unsigned int stack_pop(const hstack_t hstack, void *data_out, const unsigned int size)
{
    if (!data_out || !size || stack_valid_handler(hstack) != RESULT_OK)
        return 0;

    stack *stack = &stacks.entries[hstack].stack;
    node *cur_node = stack->top;

    if (!cur_node)
        return 0;
    assert(stack->size > 0);

    unsigned int data_size = cur_node->size;
    if (data_size > size)
        return 0;

    stack->top = cur_node->prev;
    stack->size -= 1;

    memcpy(data_out, cur_node->data, data_size);
    free(cur_node);
    return data_size;
}
