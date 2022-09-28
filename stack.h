#include <stdlib.h>
#include <assert.h>

static FILE *log_file = fopen ("log.txt", "w");

int ERRNO = 0;
static const size_t POISON = 0xDEADBEEF;
static const unsigned long long CANARY = 0xAB8EACAAAB8EACAA;

typedef double elem_t;

enum ERRORS
{
    STACK_NULL_STK              = 0x1 << 0,
    STACK_NULL_DATA             = 0x1 << 1,
    STACK_STACK_OVERFLOW        = 0x1 << 2,
    STACK_INCORRECT_SIZE        = 0x1 << 3,
    STACK_DATA_LIMITS_VIOLATED  = 0x1 << 4,
    STACK_STACK_LIMITS_VIOLATED = 0x1 << 5
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
    unsigned long long stack_start = CANARY;

    struct Debug_info info = {};

    elem_t *data = nullptr;

    int size = 0;
    int capacity = 0;

    unsigned long long stack_end = CANARY;
};


void   stack_init (Stack *stk, int capacity);
int    stack_push (Stack *stk, elem_t value, int *err = &ERRNO);
elem_t stack_pop  (Stack *stk,               int *err = &ERRNO);

void   __debug_stack_init (Stack *stk, int capacity, const char *var_stk,   const char *call_func,
                                                     const char *call_file, const int creat_line);
int    __debug_stack_push (Stack *stk, elem_t value, const int call_line, int *err = &ERRNO);
elem_t __debug_stack_pop  (Stack *stk,               const int call_line, int *err = &ERRNO);

void   stack_realloc (Stack *stk, int previous_capacity);
void   fill_stack    (Stack *stk, int start);
int    stack_error   (Stack *stk, int *err);
void   stack_dump    (Stack *stk, int *err);
void   stack_resize  (Stack *stk);
void   stack_dtor    (Stack *stk);

void   log_sostoyanie   (Stack *stk, int *err);
void   log_info         (Stack *stk, int *err);
void   log_data         (Stack *stk);
void   log_data_members (Stack *stk);


void stack_realloc (Stack *stk, int previous_capacity)
{
    if (previous_capacity)
    {
        stk->data = (elem_t *)((char *)stk->data - sizeof (CANARY));
        stk->data = (elem_t *)realloc (stk->data, stk->capacity * sizeof (elem_t) + 2 * sizeof (CANARY));

        assert (stk->data);

        stk->data = (elem_t *)((char *)stk->data + sizeof (CANARY));

        *((unsigned long long *)(stk->data + previous_capacity)) = 0;
        *((unsigned long long *)(stk->data + stk->capacity)) = CANARY;
    }
    else
    {
        stk->data = (elem_t *)calloc (1, stk->capacity * sizeof (elem_t) + 2 * sizeof (CANARY));

        assert (stk->data);

        *((unsigned long long *)(stk->data)) = CANARY;

        stk->data = (elem_t *)(((char *)(stk->data)) + sizeof (CANARY));

        *((unsigned long long *)(stk->data + stk->capacity)) = CANARY;
    }
}

void fill_stack (Stack *stk, int start)
{
    for (int i = start - 1; i < stk->capacity; i++)
    {
        (stk->data)[i] = POISON;
        printf ("\n[%llu]\n", (stk->data)[i]);
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

    (stk->data)[stk->size] = value;
    stk->size += 1;
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

    stack_realloc (stk, 0);

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
    if ((*((unsigned long long *)((char*)stk->data - sizeof (CANARY))) != CANARY) ||
        (*((unsigned long long *)(stk->data + stk->capacity)) != CANARY))
    {
        *err |= STACK_DATA_LIMITS_VIOLATED;
    }
    if (stk->stack_start != CANARY || stk->stack_end != CANARY)
    {
        *err |= STACK_STACK_LIMITS_VIOLATED;
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
    stk->data =(elem_t *)((char *)stk->data - 8);

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
    const char *sostoyanie[10] = {};

    int i = 0;

    if (*err)
    {
        fprintf (log_file, "(ERROR:");

        if ((*err) & 0x1 << 0)
        {
            sostoyanie[i] = "stk is a null pointer, ";
            i++;
        }
        if ((*err) & 0x1 << 1)
        {
            sostoyanie[i] = "data is a null pointer, ";
            i++;
        }
        if ((*err) & 0x1 << 2)
        {
            sostoyanie[i] = "stack overflow, ";
            i++;
        }
        if ((*err) & 0x1 << 3)
        {
            sostoyanie[i] = "capacity or size of stack is under zero, ";
            i++;
        }
        if ((*err) & 0x1 << 4)
        {
            sostoyanie[i] = "access rights of stack data are invaded, ";
            i++;
        }
        if ((*err) & 0x1 << 5)
        {
            sostoyanie[i] = "access rights of stack are invaded";
            i++;
        }
    }
    else
    {
        sostoyanie[i] = "(ok";
        i++;
    }

    sostoyanie[i] = nullptr;

    for (int index = 0; index < i; index++)
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
        fprintf (log_file, "\t*[%ld""] = %lf\n", i, (stk->data)[i]);
    }
    for (int i = stk->size; i < stk->capacity; i++)
    {
        fprintf (log_file, "\t [%ld] = %lf\n", i, (stk->data)[i]);
    }
}

#ifdef STACK_DEBUG

#define stack_init(stk,capacity)     __debug_stack_init (stk, capacity, #stk, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#define stack_push(stk,value)        __debug_stack_push (stk, value, __LINE__)
#define stack_pop(stk)               __debug_stack_pop  (stk, __LINE__)

#endif
