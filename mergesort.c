#include <string.h>
#include <glib.h>

static void * buf = NULL;
static int buf_size = 0;

void mergesort (void * items, int n_items, int size,
                GCompareDataFunc compare, void * data)
{
    if (n_items < 2)
        return;

    int half = n_items / 2;
    int half_size = half * size;
    int full_size = n_items * size;

    if (buf_size < half_size)
    {
        buf = g_realloc (buf, half_size);
        buf_size = half_size;
    }

    mergesort (items, half, size, compare, data);
    mergesort (items + half_size, n_items - half, size, compare, data);

    memcpy (buf, items, half_size);

    const void * a = buf;
    const void * b = items + half_size;

    void * end = items + full_size;
    void * buf_end = buf + half_size;

    for (void * c = items; c < end; c += size)
    {
        if (b >= end || (a < buf_end && compare (a, b, data) < 1)) {
            memcpy (c, a, size);
            a += size;
        } else {
            memcpy (c, b, size);
            b += size;
        }
    }
}
