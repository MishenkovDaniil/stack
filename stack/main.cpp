#include <stdio.h>

#define STACK_DEBUG
#define PROT_LEVEL 3

//указатель равный малому значению
//неопределенное поведению при capacity < 10
#include "stack.h"


int main ()
{
    Stack stk1 = {};

    int errnum = 0;

    Stack *p = &stk1;

    stack_init (p, 1, &errnum);
    //stack_resize (p);
    printf ("a");

    stack_push (p, 2, &errnum);

    printf ("a");

    //stk1.info.call_line = 0;
    stack_push (&stk1, -34, &errnum);
    printf ("a");
    //stk1.info = {};
    //stk1.left_canary = 1;


    double a = stack_pop (&stk1, &errnum);
    //stk1.info = {};
    printf ("a");
    if (errnum)
    {
        printf ("error: %d\n", errnum);

        stack_dtor (p);

        return 1;
    }
    //printf ("[%lf]\t", a);
    //stk1.data[0] = 155;

    //printf ("[%lf]\t", stack_pop (&stk1));
    //printf ("[%d]\t", stack_pop (&stk1));
    //printf ("[%d]\t", stack_pop (&stk1));
    //printf ("[%lf]\t", stack_pop (&stk1));
    //printf ("[%d]\t", stack_pop (&stk1));
    //printf ("[%d]\t", stack_pop (&stk1));
    //printf ("[%d]\t", stack_pop (&stk1));
    //stack_pop (&stk1);
    //stack_push (&stk1, 3);
    //stack_push (&stk1, -1);
    //stack_push (&stk1, 3);

    stack_dtor (&stk1);
    printf ("5");

    return 0;
}
