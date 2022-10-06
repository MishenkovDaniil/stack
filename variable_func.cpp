#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define printtt(...) foo (2, __VA_ARGS__)

int foo (int num, ...)
{
    int *err = nullptr;

    va_list ap;
    va_start (ap, num);

    const char *stk = va_arg (ap, const char *);

    err = va_arg (ap, int *);
    int errnn = 0;
    if (*err != 0)
    {
        err = nullptr;
        err = &errnn;
    }

    printf ("[%d]\n", *err);
    *err = 1;
    va_end (ap);
    return 0;
}

int main ()
{
    /*double *ptr = (double *)calloc (1, sizeof (double) * 5 + 2 *sizeof (unsigned long long));
    printf ("[%p]\n", ptr);

    printf ("%d", sizeof (double));

    *(unsigned long long *)(ptr + sizeof (unsigned long long)) = 12345;
    ptr = (double *)((char *)ptr + 1);

    printf ("[%p]", ptr);

    free (ptr - 1);*/

    const char *stk = "sdsd";
    printf ("%d", (int *)stk);
    int errnum = 0;

    printtt (stk);
    printf ("%d\n", errnum);
    return 0;
}
