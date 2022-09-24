#include <stdlib.h>
#include <assert.h>

static int ERRNO = 0;

typedef double elem_t;
static FILE *log_file = fopen ("log.txt", "w");

struct Debug_info
{
    const char *func      = nullptr;
    const char *file      = nullptr;
    const char *var_stk  = nullptr;
    const char *call_func = nullptr;
    const char *call_file = nullptr;
    int line = 0;
    int stk_line = -1;
};

struct Stack
{
    struct Debug_info info = {};

    elem_t *data = nullptr;

    size_t size = 0;
    size_t capacity = 0;
};

void stack_push (Stack *stk, elem_t value, const char *var_stk, const char *call_func,
                 const char *file, const int call_line);
elem_t stack_pop(Stack *stk, const char *var_stk, const char *call_func,
                 const char *call_file, const int call_line, int *err = &ERRNO);
void stack_init (Stack *stk, size_t capacity);
void stack_resize (Stack *stk);
void stack_dtor (Stack *stk);
int stack_is_ok (Stack *stk);
void log_info (FILE *log_file, Stack *stk, const char *sostoyanie);
void fill_stack (Stack *stk);
void log_data (FILE *log_file, Stack *stk, const char *sostoyanie);
void log_data_members (FILE *log_file, Stack *stk);


void fill_stack (Stack *stk, size_t start)
{
    for (size_t i = start - 1; i < stk->capacity; i++)
    {
        (stk->data)[i] = 0xDEADBEEF;
    }
}

void stack_push (Stack *stk, elem_t value, const char *var_stk, const char *call_func,
                 const char *call_file, const int call_line)
{
    (stk->info).file = __FILE__;
    (stk->info).func = __PRETTY_FUNCTION__;
    (stk->info).line = __LINE__;
    (stk->info).call_func = call_func;
    (stk->info).call_file = call_file;
    (stk->info).var_stk = var_stk;

    assert (stack_is_ok(stk));

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

    assert (stack_is_ok(stk));

/*  if (*err)
    {

    }*/
    stack_resize (stk);

    elem_t latest_value = (stk->data)[stk->size];

    stk->size--;
    (stk->data)[stk->size] = 0xDEADBEEF;

    return latest_value;
}

void stack_init (Stack *stk, size_t capacity)
{
    stk->capacity = capacity;

    stk->data = (elem_t *)calloc (stk->capacity, sizeof (elem_t));

    fill_stack (stk, 1);
}

void stack_resize (Stack *stk)
{
    size_t current_size = stk->size;
    size_t previous_capacity = stk->capacity;

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

int stack_is_ok (Stack *stk)
{

    log_info (log_file, stk, "ok");
    printf ("ok");

    return 1;
}

 ////////////////////////////////////////////////////////////////
 ////////////////////////////////////////////////////////////////
 ////////////////////////////////////////////////////////////////

void log_info (FILE *log_file, Stack *stk, const char *sostoyanie)
{
    log_data (log_file, stk, sostoyanie);

    log_data_members (log_file, stk);

    fprintf (log_file, "\n\n");
}

void log_data (FILE *log_file, Stack *stk, const char *sostoyanie)
{
    fprintf (log_file, "%s at %s(%d)\n"
                       "stack [%p](%s):\n"
                       "%s at %s in %s:\n"
                       "data [%p]:\n",
                       (stk->info).func, (stk->info).file, (stk->info).line,
                        stk, sostoyanie, (stk->info).var_stk, stk->info.call_func, stk->info.call_file, stk->data);
}

void log_data_members (FILE *log_file, Stack *stk)
{
    for (int i = 0; i < stk->capacity; i++)
    {
        fprintf (log_file, "\t[%d] = %lf\n", i, (stk->data)[i]);
    }
}
