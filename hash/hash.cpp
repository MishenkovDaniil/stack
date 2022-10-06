#include <stdio.h>

unsigned long long m_gnu_hash (void *ptr, int size);

int main ()
{
    double str[6] = {5.0, 6.0, 5.0, 4.0};

    double *buf = str;

    printf ("%d", m_gnu_hash (buf, 4*sizeof (double)));

    return 0;
}

unsigned long long m_gnu_hash (void *ptr, int size)
{
    unsigned long long sum = 5381;

    for (unsigned long long index = 0; index < size; index++)
    {
        unsigned char byte = (((unsigned char *)ptr)[index]);
        printf ("%u\t", byte);
        printf ("%d\n", (((unsigned char *)ptr)[index]));
        sum += 33 * (((unsigned char *)ptr)[index]);
    }
    printf ("\n[%llu]\n", sum);
    return sum;
}

