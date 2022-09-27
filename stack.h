#include <stdlib.h>
#include <assert.h>

static FILE *log_file = fopen ("log.txt", "w");
int ERRNO = 0;
static const size_t POISON = 0xDEADBEEF;

typedef double elem_t;

enum ERRORS
{
    STACK_NULL_STK       = 0x1 << 0,
    STACK_NULL_DATA      = 0x1 << 1,
    STACK_STACK_OVERFLOW = 0x1 << 2,
    STACK_INCORRECT_SIZE = 0x1 << 3
};

struct Debug_info
{
    const char *func      = nullptr;
    const char *file      = nullptr;
    const char *var_stk   = nullptr;
    const char *call_func = nullptr;
    const char *call_file = nullptr;
    int creat_line = 0;
    int line = 0;
    int call_line = 0;
};

struct Stack
{
    struct Debug_info info = {};

    elem_t *data = nullptr;

    int size = 0;
    int capacity = 0;
};


void   stack_init   (Stack *stk, int capacity);
int    stack_push   (Stack *stk, elem_t value, int *err = &ERRNO);
elem_t stack_pop    (Stack *stk,               int *err = &ERRNO);

void   __debug_stack_init   (Stack *stk, int capacity, const char *var_stk,   const char *call_func,
                                                       const char *call_file, const int creat_line);
int    __debug_stack_push   (Stack *stk, elem_t value, const int call_line, int *err = &ERRNO);
elem_t __debug_stack_pop   (Stack *stk,                const int call_line, int *err = &ERRNO);

void   fill_stack   (Stack *stk, int start);
void   stack_resize (Stack *stk);
void   stack_dtor   (Stack *stk);
int    stack_error  (Stack *stk, int *err);
void   stack_dump   (Stack *stk, int *err);

void   log_info (Stack *stk, int *err);
void   log_data (Stack *stk);
void   log_data_members (Stack *stk);
void   log_sostoyanie (Stack *stk, int *err);

//int stack_push (Stack *stk, elem_t value);

/*int __debug_stack_push (Stack *stk, elem_t value, const char *func, int line)
{
    fprintf(logs, "push stack....")
    ...

    stack_push();
}*/

//#define stack_pop(stk, err) __debug_stack_pop (__func__, __LINE__, stk, value, err);
//#define stack_pop(stk) __debug_stack_pop (__func__, __LINE__, stk, value, err);

//stack_push(stack_pop());
void stack_realloc (Stack *stk)
{
    if (stk->capacity)
    {
        stk->data = (elem_t *)realloc (stk->data, stk->capacity * sizeof (elem_t));
    }
    else
    {
        stk->data = (elem_t *)calloc (stk->capacity, sizeof (elem_t));
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
        *err = 0;
    }
    if ((*err) = stack_error(stk, err))
    {
        return *err;
    }

    stack_resize (stk);

    (stk->data)[stk->size++] = value;

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
        *err = 0;
    }
    if ((*err) = stack_error(stk, err))
    {
        return (elem_t)*err;
    }

    stack_resize (stk);

    (stk->size)--;
    elem_t latest_value = (stk->data)[stk->size];

    (stk->data)[stk->size] = POISON;

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

void stack_init (Stack *stk, int capacity)
{
    if (capacity < 1)
    {
        printf ("capacity is incorrect");
        assert (0);
    }

    stk->capacity = capacity;

    stack_realloc (stk);

    fill_stack (stk, 1);
}

void __debug_stack_init (Stack *stk, int capacity, const char *var_stk, const char *call_func,
                 const char *call_file, const int creat_line)
{
    (stk->info).call_func = call_func;
    (stk->info).call_file = call_file;
    (stk->info).var_stk = var_stk;
    (stk->info).creat_line = creat_line;

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

            stack_realloc (stk);

            fill_stack (stk, previous_capacity + 1);
        }
        if (stk->capacity > current_size * 4)
        {
            stk->capacity /= 2;
            stack_realloc (stk);
        }
    }

//#define HASH_PROTEction
//#define canary_prota
/*
    if (previous_capacity != stk->capacity)
    {
    // error???
    // stack_realloc (mozhno usat kak calloc)
    // mozhet canaries postavit
        stk->data = (elem_t *)realloc (stk->data, (stk->capacity) * sizeof (elem_t));
    }*/
}


int stack_error (Stack *stk, int *err)
{
    if (stk == nullptr)
    {
        (*err) = STACK_NULL_STK;
    }
    if (stk->data == nullptr)
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

    #ifdef STACK_DEBUG
    stack_dump (stk, err);
    #endif

    if (*err)
    {
        return *err;
    }

    return 0;
}

void stack_dump (Stack *stk, int *err)
{
    log_info (stk, err);
    log_data (stk);

    fprintf (log_file, "\n\n");
}

void stack_dtor (Stack *stk)
{
    free (stk->data);
}
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void log_info (Stack *stk, int *err)
{
    fprintf (log_file,
            "%s at %s(%d)\n"
            "stack [%p]", (stk->info).func, (stk->info).file, (stk->info).line, stk);

    log_sostoyanie (stk, err);
}
void log_sostoyanie (Stack *stk, int *err)
{
    const char *sostoyanie[5] = {};

    int i = 0;

    if (*err)
    {
        fprintf (log_file, "(ERROR:");

        if ((*err) & ~(~0 << 1))
        {
            sostoyanie[i] = "stk is a null pointer,";
            i++;
        }
        if ((*err) >> 1 & ~(~0 << 2))
        {
            sostoyanie[i] = "data is a null pointer,";
            i++;
        }
        if ((*err) >> 2 & ~(~0 << 3))
        {
            sostoyanie[i] = "stack overflow,";
            i++;
        }
        if ((*err) >> 3 & ~(~0 << 4))
        {
            sostoyanie[i] = "capacity or size of stack is under zero";
            i++;
        }
    }
    else
    {
        sostoyanie[i] = "(ok";
        i++;
    }

    sostoyanie[i] = nullptr;

    for (int index = 0; sostoyanie[index] != nullptr; index++)
    {
        fprintf (log_file, "%s", sostoyanie[index]);
    }
    fprintf (log_file, ")\n");
}

void log_data (Stack *stk)
{
    fprintf (log_file,
            "%s at %s in %s(%d)(called at %d):\n"
            "data [%p]:\n",
            (stk->info).var_stk, (stk->info).call_func, (stk->info).call_file,
            (stk->info).creat_line, stk->info.call_line, stk->data);
    log_data_members (stk);
}

void log_data_members (Stack *stk)
{
    fprintf (log_file, "\tsize = %ld\n", stk->size);
    fprintf (log_file, "\tcapacity = %ld\n", stk->capacity);

    for (int i = 0; i < stk->size; i++)
    {
        fprintf (log_file, "\t*[%ld] = %lf\n", i, (stk->data)[i]);
    }
    for (int i = stk->size; i < stk->capacity; i++)
    {
        fprintf (log_file, "\t[%ld] = %lf\n", i, (stk->data)[i]);
    }
}

#ifdef STACK_DEBUG

#define stack_init(stk,capacity)     __debug_stack_init (stk, capacity, #stk, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#define stack_push(stk,value)        __debug_stack_push (stk, value, __LINE__)
#define stack_pop(stk)               __debug_stack_pop  (stk, __LINE__)

#endif
