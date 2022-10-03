#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <assert.h>

#include "hash.h"

#define CANARY_PROT 1
#define HASH_PROT 2

#ifndef PROT_LEVEL
#define PROT_LEVEL CANARY_PROT
#endif


typedef unsigned long long canary_t;
typedef double elem_t;


static int ERRNO = 0;
static const int CANARIES_NUMBER = 2;
static const size_t POISON = 0xDEADBEEF;
static const canary_t CANARY = 0xAB8EACAAAB8EACAA;

enum errors
{
    STACK_FOPEN_FAILED   = 0x1 << 0,
    STACK_ALLOC_FAIL     = 0x1 << 1,
    STACK_NULL_STK       = 0x1 << 2,
    STACK_NULL_DATA      = 0x1 << 3,
    STACK_STACK_OVERFLOW = 0x1 << 4,
    STACK_INCORRECT_SIZE = 0x1 << 5,
    STACK_VIOLATED_DATA  = 0x1 << 6,
    STACK_VIOLATED_STACK = 0x1 << 7,
    STACK_DATA_MESSED_UP = 0x1 << 8
};

struct Debug_info
{
    const char *func      = nullptr;
    const char *file      = nullptr;
    const char *stk_name  = nullptr;
    const char *call_func = nullptr;
    const char *call_file = nullptr;
    int creat_line = 0;
    int call_line = 0;
    int line = 0;
};

struct Stack
{
    #if (PROT_LEVEL & CANARY_PROT)
    canary_t left_canary = CANARY;
    #endif

    struct Debug_info info = {};

    elem_t *data = nullptr;

    int size = 0;
    int capacity = 0;

    #if (PROT_LEVEL & HASH_PROT)
    hash_t hash_sum = 0;
    #endif

    #if (PROT_LEVEL & CANARY_PROT)
    canary_t right_canary = CANARY;
    #endif
};

static FILE *log_file = fopen ("log.txt", "w");

void   stack_init (Stack *stk, int capacity, int *err = &ERRNO);
int    stack_push (Stack *stk, elem_t value, int *err = &ERRNO);
elem_t stack_pop  (Stack *stk,               int *err = &ERRNO);

void   __debug_stack_init (Stack *stk, int capacity, const char *stk_name,   const char *call_func,
                                                     const char *call_file, const int creat_line, int *err);
int    __debug_stack_push (Stack *stk, elem_t value, const int call_line, int *err);
elem_t __debug_stack_pop  (Stack *stk,               const int call_line, int *err);

static int   stack_realloc (Stack *stk, int previous_capacity, int *err = &ERRNO);
static void  fill_stack    (Stack *stk, int start, int *err);
static int   stack_error   (Stack *stk, int *err, int need_in_dump = 1);
static void  stack_dump    (Stack *stk, int *err, FILE *file = log_file);
static void  stack_resize  (Stack *stk, int *err);
void         stack_dtor    (Stack *stk);

static void   log_status       (Stack *stk, int *err, FILE *file = log_file);
static void   log_info         (Stack *stk, int *err, FILE *file = log_file);
static void   log_data         (Stack *stk, FILE *file = log_file);
static void   log_data_members (Stack *stk, FILE *file = log_file);

static int stack_realloc (Stack *stk, int previous_capacity, int *err)
{
    assert (stk);
    assert (err);

    if (!(stk && err) || previous_capacity < 0)
    {
        printf ("You couldn't use this function");
    }

    if (previous_capacity)
    {
        size_t mem_size = 0;

        #if (PROT_LEVEL & CANARY_PROT)
        stk->data = (elem_t *)((char *)stk->data - sizeof (canary_t));

        mem_size = stk->capacity * sizeof (elem_t) + CANARIES_NUMBER * sizeof (canary_t);
        #else
        mem_size = stk->capacity * sizeof (elem_t);
        #endif

        stk->data = (elem_t *)realloc (stk->data, mem_size);

        #if (PROT_LEVEL & CANARY_PROT)
        stk->data = (elem_t *)((char *)stk->data + sizeof (canary_t));

        *((canary_t *)(stk->data + previous_capacity)) = POISON;
        *((canary_t *)(stk->data + stk->capacity)) = CANARY;
        #endif
    }
    else
    {
        #if (PROT_LEVEL & CANARY_PROT)

        stk->data = (elem_t *)calloc (stk->capacity * sizeof (elem_t) + CANARIES_NUMBER * sizeof (canary_t), 1);

        *((canary_t *)(stk->data)) = CANARY;

        stk->data = (elem_t *)(((char *)(stk->data)) + sizeof (canary_t));

        *((canary_t *)(stk->data + stk->capacity)) = CANARY;

        #else

        stk->data = (elem_t *)calloc (stk->capacity, sizeof (elem_t));

        #endif
    }

    if (stk->data == nullptr)
    {
        *err |= STACK_ALLOC_FAIL;

        stack_dump(stk, err, stderr);

        return *err;
    }

    return 0;
}

void fill_stack (Stack *stk, int start, int *err)
{
    assert (stk && stk->data);

    if (!(stk && stk->data && err))
    {
        printf ("You couldn't use this function");
    }

    for (int i = start - 1; i < stk->capacity; i++)
    {
        (stk->data)[i] = POISON;
    }
}

int stack_push (Stack *stk, elem_t value, int *err)
{
    assert (stk && stk->data);
    assert (err);

    if (err == nullptr)
    {
        err = &ERRNO;
    }

    stack_error(stk, err);

    if (*err)
    {
        return *err;
    }

    stack_resize (stk, err);

    (stk->data)[stk->size++] = value;

    #if (PROT_LEVEL & HASH_PROT)

    stk->hash_sum = m_gnu_hash (stk->data, stk->capacity * sizeof(elem_t));

    #endif

    return 0;
}

int __debug_stack_push (Stack *stk, elem_t value, const int call_line, int *err)
{
    (stk->info).file = __FILE__;
    (stk->info).func = __PRETTY_FUNCTION__;
    (stk->info).line = __LINE__;
    (stk->info).call_line = call_line;

    return stack_push (stk, value, err);
}

elem_t stack_pop (Stack *stk, int *err)
{
    assert (stk && stk->data);
    assert (err);

    if (err == nullptr)
    {
        err = &ERRNO;
    }

    stack_error(stk, err);

    if (*err)
    {
        return (elem_t)*err;
    }

    stack_resize (stk, err);

    (stk->size)--;
    elem_t latest_value = (stk->data)[stk->size];

    (stk->data)[stk->size] = POISON;

    #if (PROT_LEVEL & HASH_PROT)

    stk->hash_sum = m_gnu_hash (stk->data, stk->capacity * sizeof (elem_t));

    #endif

    return latest_value;
}

elem_t __debug_stack_pop (Stack *stk, const int call_line, int *err)
{
    (stk->info).file = __FILE__;
    (stk->info).func = __PRETTY_FUNCTION__;
    (stk->info).line = __LINE__;
    (stk->info).call_line = call_line;

    return stack_pop (stk, err);
}

void stack_init (Stack *stk, int capacity, int *err)
{
    assert (stk);
    assert (err);

    if (err == nullptr)
    {
        err = &ERRNO;
    }

    stk->capacity = capacity;

    if (!(stack_realloc (stk, 0, err)))
    {
        fill_stack (stk, 1, err);

        #if (PROT_LEVEL & HASH_PROT)

        stk->hash_sum = m_gnu_hash (stk->data, stk->capacity * sizeof (elem_t));

        #endif
    }
    else
        ;

    stack_error (stk, err);
}

void __debug_stack_init (Stack *stk, int capacity, const char *stk_name, const char *call_func,
                         const char *call_file, const int creat_line, int *err)
{
    (stk->info).call_func = call_func;
    (stk->info).call_file = call_file;
    (stk->info).creat_line = creat_line;

    (stk->info).file = __FILE__;
    (stk->info).func = __PRETTY_FUNCTION__;
    (stk->info).line = __LINE__;

    (*(stk_name) != '&') ? (stk->info).stk_name = stk_name : (stk->info).stk_name = stk_name + 1;

    stack_init (stk, capacity, err);
}

static void stack_resize (Stack *stk, int *err)
{
    assert (stk && stk->data);

    if (stack_error (stk, err, 0))
    {
        stack_dump (stk, err);
    }

    int current_size = stk->size;
    int previous_capacity = stk->capacity;

    if (current_size)
    {
        if (current_size > (stk->capacity - 1))
        {
            stk->capacity *= 2;

            stack_realloc (stk, previous_capacity, err);
            fill_stack    (stk, previous_capacity + 1, err);
        }
        if (stk->capacity > current_size * 4)
        {
            stk->capacity /= 2;

            stack_realloc (stk, previous_capacity, err);
        }
    }
}

static int stack_error (Stack *stk, int *err, int need_in_dump)
{
    assert (stk && stk->data);
    assert (err);

    if (!log_file)
    {
        *err |= STACK_FOPEN_FAILED;
        stack_dump (stk, err, stderr);
    }
    if (!stk)
    {
        *err |= STACK_NULL_STK;
    }
    if (!stk->data)
    {
        *err |= STACK_NULL_DATA;
    }
    if (stk->size > stk->capacity)
    {
        *err |= STACK_STACK_OVERFLOW;
    }
    if (stk->size < 0 || stk->capacity < 0)
    {
        *err |= STACK_INCORRECT_SIZE;
    }

    #if (PROT_LEVEL & CANARY_PROT)

    if ((*((canary_t *)((char*)stk->data - sizeof (canary_t))) != CANARY) ||
        (*((canary_t *)(stk->data + stk->capacity)) != CANARY))
    {
        *err |= STACK_VIOLATED_DATA;
    }
    if (stk->left_canary != CANARY || stk->right_canary != CANARY)
    {
        *err |= STACK_VIOLATED_STACK;
    }

    #endif

    #if (PROT_LEVEL & HASH_PROT)

    if (stk->hash_sum != m_gnu_hash (stk->data, stk->capacity * sizeof (elem_t)))
    {
        *err |= STACK_DATA_MESSED_UP;
    }

    #endif

    #ifdef STACK_DEBUG
    if (need_in_dump)
    {
        stack_dump (stk, err);
    }
    #endif

    return *err;
}

static void stack_dump (Stack *stk, int *err, FILE *file)
{
    assert (stk && stk->data);
    assert (err);
    assert (file);

    if (stk && stk->data && err && file)
    {
        log_info (stk, err, file);
        log_data (stk, file);

        fprintf (file, "\n\n");
    }
    else
        ;
}

void stack_dtor (Stack *stk)
{
    assert (stk && stk->data);

    if (stk && stk->data)
    {
        stk->data = (elem_t *)((char *)stk->data - sizeof (canary_t));

        free (stk->data);
    }
    else
        ;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

static void log_info (Stack *stk, int *err, FILE *file)
{
    assert (stk && stk->data);
    assert (err);
    assert (file);

    fprintf (file,
            "%s at %s(%d)\n"
            "stack [%p]", stk->info.func, stk->info.file, stk->info.line, stk);

    log_status (stk, err, file);
}
static void log_status (Stack *stk, int *err, FILE *file)
{
    assert (stk && stk->data);
    assert (err);
    assert (file);

    const char *status[10] = {};

    int i = 0;

    if (*err)
    {
        fprintf (file, "(ERROR:");

        if (*err & STACK_FOPEN_FAILED)
        {
            status[i] = "opening log file failed";
            i++;
        }
        if (*err & STACK_ALLOC_FAIL)
        {
            status[i] = "memory allocation failed";
            i++;
        }
        if (*err & STACK_NULL_STK)
        {
            status[i] = "stk is a null pointer";
            i++;
        }
        if (*err & STACK_NULL_DATA)
        {
            status[i] = "data is a null pointer";
            i++;
        }
        if ((*err) & STACK_STACK_OVERFLOW)
        {
            status[i] = "stack overflow";
            i++;
        }
        if ((*err) & STACK_INCORRECT_SIZE)
        {
            status[i] = "capacity or size of stack is under zero";
            i++;
        }
        if ((*err) & STACK_VIOLATED_DATA)
        {
            status[i] = "access rights of stack data are invaded";
            i++;
        }
        if ((*err) & STACK_VIOLATED_STACK)
        {
            status[i] = "access rights of stack are invaded";
            i++;
        }
        if ((*err) & STACK_DATA_MESSED_UP)
        {
            status[i] = "one or more values in stack data are unexpectidly changed";
            i++;
        }
    }
    else
    {
        status[i] = "(ok";
        i++;
    }

    status[i] = nullptr;

    for (int index = 0; index < i; index++)
    {
        if (index < i - 1)
        {
            fprintf (file, "%s, ", status[index]);
        }
        else
        {
            fprintf (file, "%s", status[index]);
        }
    }
    fprintf (file, ")\n");
}

static void log_data (Stack *stk, FILE *file)
{
    assert (stk && stk->data);
    assert (file);

    fprintf (file,
            "%s at %s in %s(%d)(called at %d):\n"
            "data [%p]:\n",
            (stk->info).stk_name, (stk->info).call_func, (stk->info).call_file,
            (stk->info).creat_line, stk->info.call_line, stk->data);
    log_data_members (stk);
}

void log_data_members (Stack *stk, FILE *file)
{
    assert (stk && stk->data);
    assert (file);

    fprintf (file, "\tsize = %ld\n", stk->size);
    fprintf (file, "\tcapacity = %ld\n", stk->capacity);

    for (int i = 0; i < stk->size; i++)
    {
        fprintf (file, "\t*[%ld""] = %lf\n", i, (stk->data)[i]);
    }
    for (int i = stk->size; i < stk->capacity; i++)
    {
        fprintf (file, "\t [%ld] = %lf\n", i, (stk->data)[i]);
    }
}

#ifdef STACK_DEBUG

#define stack_init(stk,capacity,err)     __debug_stack_init (stk, capacity, #stk, __PRETTY_FUNCTION__, __FILE__, __LINE__, err)
#define stack_push(stk,value,err)        __debug_stack_push (stk, value, __LINE__, err)
#define stack_pop(stk,err)               __debug_stack_pop  (stk, __LINE__, err)

#endif

#endif /* STACK_H */
