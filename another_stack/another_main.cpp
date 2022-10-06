#include <stdio.h>

#include "another_stack.h"

int main ()
{
    Stack stk = {};

    int errrr = stack_constructor (&stk, 15);


    printf ("%d\n", errrr);
    printf ("%d\n", stk.capacity);

    int b = 0;
    //int *a = 0x20000000;

    stack_push (&stk, 6);
    stack_push (&stk, 1);
    stack_push (&stk, 6);
    //stack_pop (&stk, (int *)2);
    printf ("capacity is %d\n", stk.capacity);
    stack_push (&stk, 9);


    //errrr = stack_pop (&stk, a);

    //printf ("%d\n", *a);
    printf ("%d\n", stk.data[3]);
    printf ("%d\n", errrr);

    putchar ('t');

/*
    int *a = nullptr;
    int b = stack_pop (&stk, a);
    printf ("error is %d\n", b);
    printf ("size is %d\tcapacity is %d\n", stk.size, stk.capacity);
    char c ='f';
    stack_push (&stk, 8);
    printf ("data 0 = [%d]\t data 1 =[%d]", stk.data[0], stk.data[1]);
    putchar(c);
    printf ("a = [%d]", *a);*/

    stack_destructor (&stk);

    return 0;

}
