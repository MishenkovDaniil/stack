#include <stdlib.h>
#include <assert.h>

static int ERRNO = 0;

typedef  double elem_t;

struct Stack
{
    elem_t *data = nullptr;

    size_t size = 0;
    size_t capacity = 0;
};

void stack_push (Stack *stk, elem_t value);
elem_t stack_pop(Stack *stk);
void stack_init (Stack *stk, size_t capacity);
void stack_resize (Stack *stk);
void stack_dtor (Stack *stk);
int stack_is_ok (Stack *stk);
void log_info (FILE *log_file, Stack *stk, const char *sostoyanie);

void stack_push (Stack *stk, elem_t value)
{
    assert (stack_is_ok(stk));

    stack_resize (stk);

    (stk->data)[stk->size++] = value;
}

elem_t stack_pop(Stack *stk, int *err = &ERRNO)
{
    assert (stack_is_ok(stk));

/*  if (*err)
    {

    }*/
    stack_resize (stk);

    elem_t latest_value = (stk->data)[stk->size];
    stk->size--;

    return latest_value;
}

void stack_init (Stack *stk, size_t capacity)
{
    stk->capacity = capacity;

    stk->data = (elem_t *)calloc (stk->capacity, sizeof (elem_t));
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
    FILE *log_file = fopen ("log.txt", "w");
    log_info (log_file, stk, "ok");

    return 1;
}

void log_info (FILE *log_file, Stack *stk, const char *sostoyanie)
{
    fprintf (log_file, "%s at %s\n"
                       "stack\t [%p](%s):"
                       "", __PRETTY_FUNCTION__, __FILE__, stk,sostoyanie);
}
