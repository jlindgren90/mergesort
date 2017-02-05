#include <string.h>
#include <glib.h>

static void * buf = NULL;
static int buf_size = 0;

void do_merge (void * items, int half_size, int full_size,
               int size, GCompareDataFunc compare, void * data)
{
    if (buf_size < half_size)
    {
        buf = g_realloc (buf, half_size);
        buf_size = half_size;
    }

    memcpy (buf, items, half_size);

    const void * a = buf;
    const void * b = items + half_size;
    const void * a_end = buf + half_size;
    const void * b_end = items + full_size;

    for (void * dest = items; dest < b_end; dest += size)
    {
        if (b >= b_end || (a < a_end && compare (a, b, data) < 1)) {
            memcpy (dest, a, size);
            a += size;
        } else {
            memcpy (dest, b, size);
            b += size;
        }
    }
}

void mergesort (void * items, int n_items, int size,
                GCompareDataFunc compare, void * data)
{
    if (n_items < 2)
        return;

    int half = n_items / 2;

    mergesort (items, half, size, compare, data);
    mergesort (items + half * size, n_items - half, size, compare, data);

    do_merge (items, half * size, n_items * size,
              size, compare, data);
}
