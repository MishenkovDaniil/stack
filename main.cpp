#include <stdio.h>

#include "stack.h"

#define stack_push(A,B) stack_push (A, B, #A, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#define stack_pop(A,B)  stack_pop  (A, #A, __PRETTY_FUNCTION__, __FILE__, __LINE__, B);
#define stack_pop(A)    stack_pop  (A, #A, __PRETTY_FUNCTION__, __FILE__, __LINE__);

int main ()
{
    Stack stk1 = {};

    stack_init (&stk1, 5);
    stack_push (&stk1, 1);
    stack_push (&stk1, 2);
    stack_push (&stk1, 3);
    stack_pop (&stk1);
    stack_pop (&stk1);
    stack_push (&stk1, 2);
    stack_push (&stk1, 3);

    stack_dtor (&stk1);

    return 0;

}
