#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <glib.h>

static void * buf = NULL;
static int buf_size = 0;

void insert_single (void * head, void * end,
                    int size, GCompareDataFunc compare, void * data)
{
    uint32_t temp4;
    uint64_t temp8;
    void * dest;

    switch (size)
    {
    case 4:
        temp4 = * (uint32_t *) head;
        * (uint32_t *) head = * (uint32_t *) (head + 4);

        for (dest = head + 4; dest + 4 < end; dest += 4)
        {
            if (compare (& temp4, dest + 4, data) < 1)
                break;

            * (uint32_t *) dest = * (uint32_t *) (dest + 4);
        }

        * (uint32_t *) dest = temp4;
        break;

    case 8:
        temp8 = * (uint64_t *) head;
        * (uint64_t *) head = * (uint64_t *) (head + 8);

        for (dest = head + 8; dest + 8 < end; dest += 8)
        {
            if (compare (& temp8, dest + 8, data) < 1)
                break;

            * (uint64_t *) dest = * (uint64_t *) (dest + 8);
        }

        * (uint64_t *) dest = temp8;
        break;

    default:
        if (buf_size < size)
        {
            buf = g_realloc (buf, size);
            buf_size = size;
        }

        for (dest = head + size; dest + size < end; dest += size)
        {
            if (compare (head, dest + size, data) < 1)
                break;
        }

        memcpy (buf, head, size);
        memmove (head, head + size, dest - head);
        memcpy (dest, buf, size);
        break;
    }
}

void insertion_sort (void * items, int n_items, int size,
                     GCompareDataFunc compare, void * data)
{
    if (n_items < 2)
        return;

    void * end = items + n_items * size;
    void * head = end - size;

    while (head > items)
    {
        if (compare (head - size, head, data) > 0)
            insert_single (head - size, end, size, compare, data);

        head -= size;
    }
}

void do_merge (void * head, void * mid, void * tail,
               int size, GCompareDataFunc compare, void * data)
{
    // optimization for forward sequence
    if (compare (mid - size, mid, data) < 1)
        return;

    if (buf_size < mid - head)
    {
        buf = g_realloc (buf, mid - head);
        buf_size = mid - head;
    }

    memcpy (buf, head, mid - head);

    const void * a = buf;
    const void * a_end = buf + (mid - head);
    const void * b = mid;
    void * dest = head;

    // optimization for reverse sequence
    if (compare (head, tail - size, data) > 0)
    {
        memcpy (dest, b, tail - b);
        dest += tail - b;
        b = tail;
    }

    switch (size)
    {
    case 4:
        for (; a < a_end && b < tail; dest += 4)
        {
            if (compare (a, b, data) < 1) {
                * (int32_t *) dest = * (int32_t *) a;
                a += 4;
            } else {
                * (int32_t *) dest = * (int32_t *) b;
                b += 4;
            }
        }

        break;

    case 8:
        for (; a < a_end && b < tail; dest += 8)
        {
            if (compare (a, b, data) < 1) {
                * (int64_t *) dest = * (int64_t *) a;
                a += 8;
            } else {
                * (int64_t *) dest = * (int64_t *) b;
                b += 8;
            }
        }

        break;

    default:
        for (; a < a_end && b < tail; dest += size)
        {
            if (compare (a, b, data) < 1) {
                memcpy (dest, a, size);
                a += size;
            } else {
                memcpy (dest, b, size);
                b += size;
            }
        }

        break;
    }

    if (a < a_end)
        memcpy (dest, a, a_end - a);
}

void mergesort (void * items, int n_items, int size,
                GCompareDataFunc compare, void * data)
{
    if (n_items < 16)
    {
        insertion_sort (items, n_items, size, compare, data);
        return;
    }

    int half = n_items / 2;

    void * mid = items + half * size;
    void * tail = items + n_items * size;

    mergesort (items, half, size, compare, data);
    mergesort (mid, n_items - half, size, compare, data);

    do_merge (items, mid, tail, size, compare, data);
}

void mergesort2 (void * items, int n_items, int size,
                 GCompareDataFunc compare, void * data)
{
    if (n_items < 2)
        return;

    void * head = items + n_items * size;
    void * mid, * tail;

    void * div[64];
    int n_div = 0;

    do
    {
        mid = head;
        head = mid - size;

        while (head > items)
        {
            if (compare (head - size, head, data) > 0)
            {
                if (mid - head < 4 * size)
                    insert_single (head - size, mid, size, compare, data);
                else
                    break;
            }

            head -= size;
        }

        while (n_div >= 1)
        {
            tail = div[n_div - 1];

            if (head > items && (mid - head) <= (tail - mid) / 2)
                break;

            do_merge (head, mid, tail, size, compare, data);

            mid = tail;
            n_div --;
        }

        div[n_div] = mid;
        n_div ++;
    }
    while (head > items);
}
