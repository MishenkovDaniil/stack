#include <stdlib.h>
#include <assert.h>

static int ERRNO = 0;

typedef double elem_t;

static FILE *log_file = fopen ("log.txt", "w");

enum ERRORS
{
    STK_IS_NULLPTR = 0b01,
    DATA_IS_NULLPTR = 0b010,
    STACK_OVERLOW = 0b0100
};

struct Debug_info
{
    const char *func      = nullptr;
    const char *file      = nullptr;
    const char *var_stk   = nullptr;
    const char *call_func = nullptr;
    const char *call_file = nullptr;
    int line = 0;
    int stk_line = 0;
};

struct Stack
{
    struct Debug_info info = {};

    elem_t *data = nullptr;

    int size = 0;
    int capacity = 0;
};

void stack_push (Stack *stk, elem_t value, const char *var_stk, const char *call_func,
                 const char *file, const int call_line, int *err = &ERRNO);
elem_t stack_pop(Stack *stk, const char *var_stk, const char *call_func,
                 const char *call_file, const int call_line, int *err = &ERRNO);
void stack_init (Stack *stk, int capacity);
void stack_resize (Stack *stk);
void stack_dtor (Stack *stk);
int stack_error (Stack *stk, int *err);
void log_info (Stack *stk, int *err);
void fill_stack (Stack *stk, int start);
void log_data (Stack *stk);
void log_data_members (Stack *stk);
void stack_dump (Stack *stk, int *err);
void log_sostoyanie (Stack *stk, int *err);



void fill_stack (Stack *stk, int start)
{
    for (int i = start - 1; i < stk->capacity; i++)
    {
        (stk->data)[i] = 0xDEADBEEF;
    }
}

void stack_push (Stack *stk, elem_t value, const char *var_stk, const char *call_func,
                 const char *call_file, const int call_line, int *err)
{
    (stk->info).file = __FILE__;
    (stk->info).func = __PRETTY_FUNCTION__;
    (stk->info).line = __LINE__;
    (stk->info).call_func = call_func;
    (stk->info).call_file = call_file;
    (stk->info).var_stk = var_stk;

    if ((*err) > 0)
    {
        stack_dump (stk, err);
    }
    if ((*err) = stack_error(stk, err) > 0)
    {
        stack_dump (stk, err);
    }

    stack_resize (stk);

    (stk->data)[stk->size++] = value;
}

elem_t stack_pop(Stack *stk, const char *var_stk, const char *call_func,
                 const char *call_file, const int call_line,  int *err)
{
    (stk->info).file = __FILE__;
    (stk->info).func = __PRETTY_FUNCTION__;
    (stk->info).line = __LINE__;
    (stk->info).call_func = call_func;
    (stk->info).call_file = call_file;
    (stk->info).var_stk = var_stk;

    if ((*err) > 0)
    {
        stack_dump (stk, err);
    }
    if ((*err) = stack_error(stk, err) > 0)
    {
        stack_dump (stk, err);
    }

/*  if (*err)
    {

    }*/
    stack_resize (stk);

    elem_t latest_value = (stk->data)[stk->size];

    stk->size--;
    (stk->data)[stk->size] = 0xDEADBEEF;

    return latest_value;
}

void stack_init (Stack *stk, int capacity)
{
    stk->capacity = capacity;

    stk->data = (elem_t *)calloc (stk->capacity, sizeof (elem_t));

    fill_stack (stk, 1);
}

void stack_resize (Stack *stk)
{
    int current_size = stk->size;
    int previous_capacity = stk->capacity;

    if (current_size > 1)
    {
        while (current_size > stk->capacity)
        {
            stk->capacity *= 2;

            fill_stack (stk, stk->capacity / 2);
        }
        while (stk->capacity > current_size * 4)
        {
            stk->capacity /= 2;
        }
    }

    if (previous_capacity != stk->capacity)
    {
        stk->data = (elem_t *)realloc (stk->data, stk->capacity * sizeof (elem_t));
    }
}

void stack_dtor (Stack *stk)
{
    free (stk->data);
}

int stack_error (Stack *stk, int *err)
{
    if (stk == nullptr)
    {
        (*err) = STK_IS_NULLPTR;
    }
    if (stk->data == nullptr)
    {
        *err |= DATA_IS_NULLPTR;
    }
    if (stk->size > stk->capacity)
    {
        *err |= STACK_OVERLOW;
        printf ("1");
    }
    if (stk->size < 0 || stk->capacity < 0)
    {
        *err |= 0b01000;
    }
    stack_dump (stk, err);

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
    /*if (*err)
        exit(0);*/
}

////////////////////////////////////////////////////////////////
 ////////////////////////////////////////////////////////////////
 ////////////////////////////////////////////////////////////////
void log_info (Stack *stk, int *err)
{
    fprintf (log_file,
            "%s at %s(%d)\n"
            "stack [%p]",
            (stk->info).func, (stk->info).file, (stk->info).line,
            stk);
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
            "%s at %s in %s:\n"
            "data [%p]:\n",
            (stk->info).var_stk, stk->info.call_func, stk->info.call_file, stk->data);
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
