/*
 * Adaptive Merge Sort
 * Copyright 2017 John Lindgren
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions, and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions, and the following disclaimer in the documentation
 *    provided with the distribution.
 *
 * This software is provided "as is" and without any warranty, express or
 * implied. In no event shall the authors be liable for any damages arising from
 * the use of this software.
 */

/*
 * This algorithm borrows some ideas from TimSort but is not quite as
 * sophisticated.  Runs are detected, but only in the forward direction, and the
 * invariant is stricter: each stored run must be no more than half the length
 * of the previous.
 *
 * As in TimSort, an already-sorted array will be processed in linear time,
 * making this an "adaptive" algorithm.
 */

#include "mergesort.h"

#include <stdint.h>
#include <string.h>
#include <glib.h>

/* Inserts a single element into a sorted list */

static void insert_head (void * head, void * tail,
                         int size, CompareFunc compare, void * context,
                         void * * buf, int * buf_size)
{
    uint32_t temp4;
    uint64_t temp8;
    void * dest;

    switch (size)
    {
    case 4:
        /* optimized version for 32-bit word */
        temp4 = * (uint32_t *) head;
        * (uint32_t *) head = * (uint32_t *) (head + 4);

        for (dest = head + 4; dest + 4 < tail; dest += 4)
        {
            if (compare (& temp4, dest + 4, context) < 1)
                break;

            * (uint32_t *) dest = * (uint32_t *) (dest + 4);
        }

        * (uint32_t *) dest = temp4;
        break;

    case 8:
        /* optimized version for 64-bit word */
        temp8 = * (uint64_t *) head;
        * (uint64_t *) head = * (uint64_t *) (head + 8);

        for (dest = head + 8; dest + 8 < tail; dest += 8)
        {
            if (compare (& temp8, dest + 8, context) < 1)
                break;

            * (uint64_t *) dest = * (uint64_t *) (dest + 8);
        }

        * (uint64_t *) dest = temp8;
        break;

    default:
        /* generic version */
        if (* buf_size < size)
        {
            * buf = g_realloc (* buf, size);
            * buf_size = size;
        }

        for (dest = head + size; dest + size < tail; dest += size)
        {
            if (compare (head, dest + size, context) < 1)
                break;
        }

        memcpy (* buf, head, size);
        memmove (head, head + size, dest - head);
        memcpy (dest, * buf, size);
        break;
    }
}

/* Merges two sorted sub-lists */

static void do_merge (void * head, void * mid, void * tail,
                      int size, CompareFunc compare, void * context,
                      void * * buf, int * buf_size)
{
    if (* buf_size < mid - head)
    {
        * buf = g_realloc (* buf, mid - head);
        * buf_size = mid - head;
    }

    /* copy list "a" to temporary storage */
    memcpy (* buf, head, mid - head);

    const void * a = * buf;
    const void * a_end = a + (mid - head);
    const void * b = mid;
    void * dest = head;

    /* Handle the case of strictly separate (but reversed) lists specially.
     * In this case, we simply shift list "b" to the left. */
    if (compare (a, tail - size, context) > 0)
    {
        memmove (dest, b, tail - b);
        dest += tail - b;
        b = tail;
    }

    /* intersperse elements */
    switch (size)
    {
    case 4:
        /* optimized version for 32-bit word */
        for (; a < a_end && b < tail; dest += 4)
        {
            if (compare (a, b, context) < 1) {
                * (int32_t *) dest = * (int32_t *) a;
                a += 4;
            } else {
                * (int32_t *) dest = * (int32_t *) b;
                b += 4;
            }
        }

        break;

    case 8:
        /* optimized version for 64-bit word */
        for (; a < a_end && b < tail; dest += 8)
        {
            if (compare (a, b, context) < 1) {
                * (int64_t *) dest = * (int64_t *) a;
                a += 8;
            } else {
                * (int64_t *) dest = * (int64_t *) b;
                b += 8;
            }
        }

        break;

    default:
        /* generic version */
        for (; a < a_end && b < tail; dest += size)
        {
            if (compare (a, b, context) < 1) {
                memcpy (dest, a, size);
                a += size;
            } else {
                memcpy (dest, b, size);
                b += size;
            }
        }

        break;
    }

    /* copy remainder of list "a" */
    if (a < a_end)
        memcpy (dest, a, a_end - a);
}

/* Top-level merge sort algorithm */

void mergesort (void * items, int n_items, int size,
                CompareFunc compare, void * context)
{
    /* A list with 0 or 1 element is sorted by definition. */
    if (n_items < 2)
        return;

    void * buf = NULL;
    int buf_size = 0;

    /* The algorithm runs right-to-left (so that insertions are left-to-right). */
    void * head = items + n_items * size;
    void * mid, * tail, * tail2;

    /* Markers recording the divisions between sorted sub-lists or "runs".
     * Each run is at least 2x the length of its left-hand neighbor, so in
     * theory a list of 2^64 - 1 elements will have no more than 64 runs. */
    void * div[64];
    int n_div = 0;

    do
    {
        mid = head;
        head = mid - size;

        /* Scan right-to-left to find a run of increasing values.
         * If necessary, use insertion sort to create a run at 4 values long.
         * At this scale, insertion sort is faster due to lower overhead. */
        while (head > items)
        {
            if (compare (head - size, head, context) > 0)
            {
                if (mid - head < 4 * size)
                    insert_head (head - size, mid, size, compare, context, & buf, & buf_size);
                else
                    break;
            }

            head -= size;
        }

        /* Merge/collapse sub-lists left-to-right to maintain the invariant. */
        while (n_div >= 1)
        {
            tail = div[n_div - 1];

            while (n_div >= 2)
            {
                tail2 = div[n_div - 2];

                /*
                 * Check for the occasional case where the new sub-list is
                 * longer than both the two previous.  In this case, a "3-way"
                 * merge is performed as follows:
                 *
                 *   |---------- #6 ----------|- #5 -|---- #4 ----| ...
                 *
                 * First, the two previous sub-lists (#5 and #4) are merged.
                 * (This is more balanced and therefore more efficient than
                 * merging the long #6 with the short #5.)
                 *
                 *   |---------- #5 ----------|-------- #4 -------| ...
                 *
                 * The invariant guarantees that the newly merged sub-list (#4)
                 * will be shorter than its right-hand neighbor (#3).
                 *
                 * At this point we loop, and one of two things can happen:
                 *
                 *  1) If sub-list #5 is no longer than #3, we drop out of the
                 *     loop.  #5 is still longer than half of #4, so a 2-way
                 *     merge will be required to restore the invariant.
                 *
                 *  2) If #5 is longer than even #3 (rare), we perform another
                 *     3-way merge, starting with #4 and #3.  The same result
                 *     holds true: the newly merged #3 will again be shorter
                 *     than its right-hand neighbour (#2).  In this fashion the
                 *     process can be continued down the line with no more than
                 *     two sub-lists violating the invariant at any given time.
                 *     Eventually no more 3-way merges can be performed, and the
                 *     invariant is restored by a final 2-way merge.
                 */

                if ((mid - head) <= (tail2 - tail))
                    break;

                do_merge (mid, tail, tail2, size, compare, context, & buf, & buf_size);

                tail = tail2;
                n_div --;
            }

            /*
             * Otherwise, check whether the new sub-list is longer than half its
             * right-hand neighbour.  If so, merge the two sub-lists.  The
             * merged sub-list may in turn be longer than its own right-hand
             * neighbor, and if so the entire process is repeated.
             *
             * Once the "head" pointer reaches the beginning of the original
             * list, we simply keep merging until only one sub-list remains.
             */

            if (head > items && (mid - head) <= (tail - mid) / 2)
                break;

            do_merge (head, mid, tail, size, compare, context, & buf, & buf_size);

            mid = tail;
            n_div --;
        }

        /* push the new sub-list onto the stack */
        div[n_div] = mid;
        n_div ++;
    }
    while (head > items);

    /* release any temporary storage used */
    g_free (buf);
}
