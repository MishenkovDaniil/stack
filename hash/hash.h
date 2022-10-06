#ifndef HASH_H
#define HASH_H

#include <assert.h>

typedef unsigned long long hash_t;

static hash_t m_gnu_hash (void *ptr, int size);


static hash_t m_gnu_hash (void *ptr, int size)
{
    assert (ptr);

    if (!ptr)
    {
        return 0;
    }


    hash_t sum = 5381;

    for (hash_t index = 0; index < size; index++)
    {
        sum = 33 * sum + ((char *)ptr)[index];
    }

    return sum;
}

#endif /* HASH_H */
