#include <stdlib.h>
#include <assert.h>

#define CANARY_PROT 1
#define HASH_PROT 2

#ifndef PROT_LEVEL
#define PROT_LEVEL CANARY_PROT
#endif

typedef unsigned long long canary_t;
typedef unsigned long long hash_t;
typedef double elem_t;

static int ERRNO = 0;
static const size_t POISON = 0xDEADBEEF;
static const canary_t CANARY = 0xAB8EACAAAB8EACAA;


enum ERRORS
{
    STACK_FOPEN_FAILED   = 0x1 << 0,
    STACK_NULL_STK       = 0x1 << 1,
    STACK_NULL_DATA      = 0x1 << 2,
    STACK_STACK_OVERFLOW = 0x1 << 3,
    STACK_INCORRECT_SIZE = 0x1 << 4,
    STACK_VIOLATED_DATA  = 0x1 << 5,
    STACK_VIOLATED_STACK = 0x1 << 6,
    STACK_DATA_MESSED_UP = 0x1 << 7
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

void   stack_realloc (Stack *stk, int previous_capacity);
void   calloc_error  (void *ptr);
void   fill_stack    (Stack *stk, int start);
void   stack_error   (Stack *stk, int *err);
void   stack_dump    (Stack *stk, int *err);
void   stack_resize  (Stack *stk);
void   stack_dtor    (Stack *stk);

void   log_status   (Stack *stk, int *err);
void   log_info         (Stack *stk, int *err);
void   log_data         (Stack *stk);
void   log_data_members (Stack *stk);

hash_t m_gnu_hash (void *ptr, int size);

void calloc_error (void *ptr)
{
    if (ptr == nullptr)
    {
        fprintf (stderr, "memory allocation crashed");
    }
}

void stack_realloc (Stack *stk, int previous_capacity)
{
    if (previous_capacity)
    {
        #if (PROT_LEVEL & CANARY_PROT)

        stk->data = (elem_t *)((char *)stk->data - sizeof (canary_t));
        stk->data = (elem_t *)realloc (stk->data, stk->capacity * sizeof (elem_t) + 2 * sizeof (canary_t));

        assert (stk->data);

        stk->data = (elem_t *)((char *)stk->data + sizeof (canary_t));

        *((canary_t *)(stk->data + previous_capacity)) = POISON;
        *((canary_t *)(stk->data + stk->capacity)) = CANARY;

        #else

        stk->data = (elem_t *)realloc (stk->data, stk->capacity * sizeof (elem_t));

        assert (stk->data);

        #endif

        calloc_error (stk->data);
    }
    else
    {
        #if (PROT_LEVEL & CANARY_PROT)

        stk->data = (elem_t *)calloc (1, stk->capacity * sizeof (elem_t) + 2 * sizeof (canary_t));


        *((canary_t *)(stk->data)) = CANARY;

        stk->data = (elem_t *)(((char *)(stk->data)) + sizeof (canary_t));

        *((canary_t *)(stk->data + stk->capacity)) = CANARY;

        #else

        stk->data = (elem_t *)calloc (stk->capacity, sizeof (elem_t));

        #endif

        calloc_error (stk->data);
    }
}

void fill_stack (Stack *stk, int start)
{
    for (int i = start - 1; i < stk->capacity; i++)
    {
        (stk->data)[i] = POISON;
    }
}

int stack_push (Stack *stk, elem_t value, int *err)
{
    if (err == nullptr)
    {
        err = &ERRNO;
    }

    stack_error(stk, err);

    if (*err)
    {
        return *err;
    }

    stack_resize (stk);

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
    if (err == nullptr)
    {
        err = &ERRNO;
    }

    stack_error(stk, err);

    if (*err)
    {
        return (elem_t)*err;
    }

    stack_resize (stk);

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
    stack_error (stk, err);

    stk->capacity = capacity;

    stack_realloc (stk, 0);

    fill_stack (stk, 1);

    #if (PROT_LEVEL & HASH_PROT)

    stk->hash_sum = m_gnu_hash (stk->data, stk->capacity * sizeof (elem_t));

    #endif
}

void __debug_stack_init (Stack *stk, int capacity, const char *stk_name, const char *call_func,
                         const char *call_file, const int creat_line, int *err)
{
    (stk->info).call_func = call_func;
    (stk->info).call_file = call_file;
    (stk->info).creat_line = creat_line;

    (*(stk_name) != '&') ? (stk->info).stk_name = stk_name : (stk->info).stk_name = stk_name + 1;

    stack_init (stk, capacity);
}

void stack_resize (Stack *stk)
{
    int current_size = stk->size;
    int previous_capacity = stk->capacity;

    if (current_size)
    {
        if (current_size > (stk->capacity - 1))
        {
            stk->capacity *= 2;

            stack_realloc (stk, previous_capacity);
            fill_stack    (stk, previous_capacity + 1);
        }
        if (stk->capacity > current_size * 4)
        {
            stk->capacity /= 2;

            stack_realloc (stk, previous_capacity);
        }
    }
}

hash_t m_gnu_hash (void *ptr, int size)
{
    hash_t sum = 5381;

    for (hash_t index = 0; index < size; index++)
    {
        sum = 33 * sum + ((char *)ptr)[index];
    }

    return sum;
}

void stack_error (Stack *stk, int *err)
{
    if (!log_file)
    {
        *err |= STACK_FOPEN_FAILED;
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
    if (stk->size < 0 || stk->capacity < 1)
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
    stack_dump (stk, err);
    #endif
}

void stack_dump (Stack *stk, int *err)
{
    log_info (stk, err);
    log_data (stk);

    fprintf (log_file, "\n\n");
}

void stack_dtor (Stack *stk)
{
    stk->data = (elem_t *)((char *)stk->data - sizeof (canary_t));

    if (stk->data)
    {
        free (stk->data);
    }
    else
        ;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void log_info (Stack *stk, int *err)
{
    if (*err & STACK_FOPEN_FAILED)
    {
        log_file = stderr;
    }

    fprintf (log_file,
            "%s at %s(%d)\n"
            "stack [%p]", stk->info.func, stk->info.file, stk->info.line, stk);

    log_status (stk, err);
}
void log_status (Stack *stk, int *err)
{
    const char *status[10] = {};

    int i = 0;

    if (*err)
    {
        fprintf (log_file, "(ERROR:");

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
            fprintf (log_file, "%s, ", status[index]);
        }
        else
        {
            fprintf (log_file, "%s", status[index]);
        }
    }
    fprintf (log_file, ")\n");
}

void log_data (Stack *stk)
{
    fprintf (log_file,
            "%s at %s in %s(%d)(called at %d):\n"
            "data [%p]:\n",
            (stk->info).stk_name, (stk->info).call_func, (stk->info).call_file,
            (stk->info).creat_line, stk->info.call_line, stk->data);
    log_data_members (stk);
}

void log_data_members (Stack *stk)
{
    fprintf (log_file, "\tsize = %ld\n", stk->size);
    fprintf (log_file, "\tcapacity = %ld\n", stk->capacity);

    for (int i = 0; i < stk->size; i++)
    {
        fprintf (log_file, "\t*[%ld""] = %lf\n", i, (stk->data)[i]);
    }
    for (int i = stk->size; i < stk->capacity; i++)
    {
        fprintf (log_file, "\t [%ld] = %lf\n", i, (stk->data)[i]);
    }
}

#ifdef STACK_DEBUG

#define stack_init(stk,capacity,err)     __debug_stack_init (stk, capacity, #stk, __PRETTY_FUNCTION__, __FILE__, __LINE__, err)
#define stack_push(stk,value,err)        __debug_stack_push (stk, value, __LINE__, err)
#define stack_pop(stk,err)               __debug_stack_pop  (stk, __LINE__, err)

#endif
