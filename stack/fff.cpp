/**
 *\file
 */

#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <assert.h>
#include <windows.h>

#define CANARY_PROT 1 // state value for turning on canary protection of stack and stack data
#define HASH_PROT 2   // state value for turning on hash protection of stack and stack data

#ifndef PROT_LEVEL
#define PROT_LEVEL CANARY_PROT
#endif

#if (PROT_LEVEL & HASH_PROT)
#include "..\hash\hash.h"
#endif

typedef unsigned long long canary_t; // sets canary type
typedef int elem_t;               // sets type of data elements


static int ERRNO = 0;                              // sets a "non-error" value
static const int CANARIES_NUMBER = 2;              // sets a number of "canaries"
static const size_t POISON = 0xDEADBEEF;           // sets "poison" value (a value to indicate errors in stack data values)
static const canary_t CANARY = 0xAB8EACAAAB8EACAA; // sets value of "canary" (a value to indicate safety of stack and stack data)
static const int START_CAPACITY = 10;

enum errors
{
    STACK_FOPEN_FAILED   = 0x1 << 0,
    STACK_ALLOC_FAIL     = 0x1 << 1,
    STACK_BAD_READ_STK       = 0x1 << 2,
    STACK_BAD_READ_DATA      = 0x1 << 3,
    STACK_STACK_OVERFLOW = 0x1 << 4,
    STACK_INCORRECT_SIZE = 0x1 << 5,
    STACK_VIOLATED_DATA  = 0x1 << 6,
    STACK_VIOLATED_STACK = 0x1 << 7,
    STACK_DATA_MESSED_UP = 0x1 << 8
};

struct Debug_info
{
    const char *func      = nullptr; // name of called function
    const char *file      = nullptr; // name of file where the called function is
    const char *stk_name  = nullptr; // name of stack
    const char *call_func = nullptr; // name of calling function
    const char *call_file = nullptr; // name of file where the calling function is
    int creat_line = 0;              // line where stack is created (initialised)
    int call_line = 0;               // line where calling function is situated
    int line = 0;                    // line where called function starts
};

/// struct with info about stack
struct Stack
{
    #if (PROT_LEVEL & CANARY_PROT)
    canary_t left_canary = CANARY; // "canary" to avoid foreign data contamination of stack
    #endif

    struct Debug_info info = {};

    elem_t *data = nullptr;

    int size = 0;                  // number of initialised elements in data
    int capacity = 0;

    #if (PROT_LEVEL & HASH_PROT)
    hash_t hash_sum = 0;
    #endif

    #if (PROT_LEVEL & CANARY_PROT)
    canary_t right_canary = CANARY; // "canary" to avoid foreign data contamination of stack
    #endif
};

static FILE *log_file = fopen ("log.txt", "w"); // output file

/**
 *creates stack data
 * \param [out
 ] stk      pointer to struct Stack
 * \param [in] capacity start capacity for data
 * \param [in] err      show if situation error or not error
 * \return              null if success, else error code
 */
int   stack_init (Stack *stk, int capacity, int *err = &ERRNO);

/**
 *push value in stack data
 * \param [out] stk      pointer to struct Stack
 * \param [in] value    value to push
 * \param [in] err      show if situation error or not error
 * \return              null if success, else error code
 */
int    stack_push (Stack *stk, elem_t value, int *err = &ERRNO);

/**
 *pop latest element of data
 * \param [out] stk      pointer to struct Stack
 * \param [in] err      show if situation error or not error
 * \return              latest element of data
 */
elem_t stack_pop  (Stack *stk,               int *err = &ERRNO);

int   __debug_stack_init (Stack *stk, int capacity, const char *stk_name, const char *call_func, const int call_line,
                                                     const char *call_file, const int creat_line, int *err = &ERRNO);
int    __debug_stack_push (Stack *stk, elem_t value, const int call_line, int *err = &ERRNO);
elem_t __debug_stack_pop  (Stack *stk,               const int call_line, int *err = &ERRNO);

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
int is_bad_read_ptr (void *p);


int is_bad_read_ptr (void *p)
{
    MEMORY_BASIC_INFORMATION mbi = {};

    const int PROTECT_MASK = PAGE_EXECUTE | PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
    if (!(VirtualQuery (p, &mbi, sizeof (mbi))))
    {
        return 1;
    }
    else if (!(mbi.Protect & PROTECT_MASK))
    {
        return 1;
    }

    return 0;
}

static int stack_realloc (Stack *stk, int previous_capacity, int *err)
{
    assert (stk);
    assert (err);

    if (is_bad_read_ptr(stk) || err == nullptr || previous_capacity < 0)
    {
        printf ("ERROR: stack pointer or error pointer is a nullptr or previous capacity at func stack_realloc is under zero\n");
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

        #ifdef STACK_DEBUG
        stack_dump(stk, err, stderr);
        #endif

        return *err;
    }

    return 0;
}

void fill_stack (Stack *stk, int start, int *err)
{
    assert (stk && stk->data);

    if (!(stk && stk->data && err))
    {
        printf ("ERROR: stack, stack data or error pointer is a null pointer\n");
    }

    for (int i = start; i < stk->capacity; i++)
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

    stack_error (stk, err);

    if (*err)
    {
        return *err;
    }

    stack_resize (stk, err);

    (stk->data)[stk->size++] = value;

    #if (PROT_LEVEL & HASH_PROT)

    stk->hash_sum = m_gnu_hash (stk->data, stk->capacity * sizeof(elem_t));

    #endif

    stack_error (stk, err);

    return 0;
}

int __debug_stack_push (Stack *stk, elem_t value, const int call_line, int *err)
{
    if (stack_error (stk, err, 0))
    {
        #ifdef STACK_DEBUG
        stack_dump (stk, err);
        #endif

        return POISON;
    }
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

    stack_error (stk, err);

    if (*err)
    {
        return (elem_t)*err;
    }

    stack_resize (stk, err);

    (stk->size)--;

    if (stk->size < 0)
    {
        stack_error (stk, err);

        return POISON;
    }

    elem_t latest_value = (stk->data)[stk->size];

    (stk->data)[stk->size] = POISON;

    #if (PROT_LEVEL & HASH_PROT)

    stk->hash_sum = m_gnu_hash (stk->data, stk->capacity * sizeof (elem_t));

    #endif

    stack_error(stk, err);

    return latest_value;
}

elem_t __debug_stack_pop (Stack *stk, const int call_line, int *err)
{
    if (stack_error (stk, err, 0))
    {
        #ifdef STACK_DEBUG
        stack_dump (stk, err);
        #endif

        return POISON;
    }
    (stk->info).file = __FILE__;
    (stk->info).func = __PRETTY_FUNCTION__;
    (stk->info).line = __LINE__;
    (stk->info).call_line = call_line;

    return stack_pop (stk, err);
}

int stack_init (Stack *stk, int capacity, int *err)
{
    assert (stk);
    assert (err);

    if (err == nullptr)
    {
        err = &ERRNO;
    }

    if (is_bad_read_ptr (stk))
    {
        stack_error (stk, err);
    }

    stk->capacity = capacity;

    if (!(stack_realloc (stk, 0, err)))
    {
        fill_stack (stk, 0, err);

        #if (PROT_LEVEL & HASH_PROT)

        stk->hash_sum = m_gnu_hash (stk->data, stk->capacity * sizeof (elem_t));

        #endif
    }

    stack_error (stk, err);

    return 0;
}

int __debug_stack_init (Stack *stk, int capacity, const char *stk_name, const char *call_func, const int call_line,
                         const char *call_file, const int creat_line, int *err)
{
    if (is_bad_read_ptr (stk))
    {
        stack_error (stk, err);
    }
    (stk->info).call_func = call_func;
    (stk->info).call_file = call_file;
    (stk->info).creat_line = creat_line;
    stk->info.call_line = call_line;

    (stk->info).file = __FILE__;
    (stk->info).func = __PRETTY_FUNCTION__;
    (stk->info).line = __LINE__;

    (*(stk_name) != '&') ? (stk->info).stk_name = stk_name : (stk->info).stk_name = stk_name + 1;

    return stack_init (stk, capacity, err);
}

static void stack_resize (Stack *stk, int *err)
{
    assert (stk && stk->data);

    if (stack_error (stk, err, 0))
    {
        #ifdef STACK_DEBUG
        stack_dump (stk, err);
        #endif
    }

    int current_size = stk->size;
    int previous_capacity = stk->capacity;
/*
    if (stk->capacity < START_CAPACITY)
    {
        stk->capacity = START_CAPACITY;
    }
    else */if (current_size)
    {
        if (current_size > (stk->capacity - 1))
        {
            stk->capacity *= 2;

            stack_realloc (stk, previous_capacity, err);
            fill_stack    (stk, previous_capacity, err);
        }
        if (stk->capacity > current_size * 4)
        {
            stk->capacity /= 2;

            stack_realloc (stk, previous_capacity, err);
        }
    }

    if (stack_error (stk, err, 0) != STACK_DATA_MESSED_UP)
    {
        #ifdef STACK_DEBUG
        stack_dump (stk, err);
        #endif
    }
    else
    {
        *err = 0;
    }
}

static int stack_error (Stack *stk, int *err, int need_in_dump)
{
    assert (stk);
    assert (err);

    do
    {
    if (!log_file)
    {
        *err |= STACK_FOPEN_FAILED;

        break;
    }
    if (is_bad_read_ptr (stk))
    {
        fprintf (stderr, "stk is a bad ptr\n");
        *err |= STACK_BAD_READ_STK;

        break;
    }
    if (is_bad_read_ptr (stk->data))
    {
        *err |= STACK_BAD_READ_DATA;

        break;
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
    } while (0);

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
    if (*err & STACK_BAD_READ_DATA)
    {
        fprintf (stderr, "data is a bad ptr\n");
    }

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

        stk->data = nullptr;
        stk = nullptr;
    }
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

    int err_number = 10;

    const char **status = (const char **)calloc (err_number, sizeof (const char *));

    int index = 0;

    if (*err)
    {
        fprintf (file, "(ERROR:");

        if (*err & STACK_FOPEN_FAILED)
        {
            status[index++] = "opening log file failed";
        }
        if (*err & STACK_ALLOC_FAIL)
        {
            status[index++] = "memory allocation failed";
        }
        if ((*err) & STACK_STACK_OVERFLOW)
        {
            status[index++] = "stack overflow";
        }
        if ((*err) & STACK_INCORRECT_SIZE)
        {
            status[index++] = "capacity or size of stack is under zero";
        }
        if ((*err) & STACK_VIOLATED_DATA)
        {
            status[index++] = "access rights of stack data are invaded";
        }
        if ((*err) & STACK_VIOLATED_STACK)
        {
            status[index++] = "access rights of stack are invaded";
        }
        if ((*err) & STACK_DATA_MESSED_UP)
        {
            status[index++] = "one or more values in stack data are unexpectidly changed";
        }
    }
    else
    {
        status[index++] = "(ok";
    }

    status[index] = nullptr;

    for (int i = 0; i < index; i++)
    {
        if (i < index - 1)
        {
            fprintf (file, "%s, ", status[i]);
        }
        else
        {
            fprintf (file, "%s", status[i]);
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

#define stack_init(stk, capacity)     __debug_stack_init (stk, capacity, #stk, __PRETTY_FUNCTION__, __LINE__, __FILE__, __LINE__)
#define stack_push(stk, value)        __debug_stack_push (stk, value, __LINE__)
#define stack_pop(stk)               __debug_stack_pop  (stk, __LINE__)

#endif

#endif /* STACK_H */
