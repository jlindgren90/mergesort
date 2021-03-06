/*
 * Adaptive Merge Sort
 * Copyright 2017-2019 John Lindgren
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

#ifndef MERGESORT_H
#define MERGESORT_H

#include <algorithm>
#include <iterator>
#include <vector>

/*
 * This algorithm borrows some ideas from TimSort but is not quite as
 * sophisticated.  Runs are detected, but only in the forward direction, and the
 * invariant is stricter: each stored run must be no more than half the length
 * of the previous.
 *
 * As in TimSort, an already-sorted array will be processed in linear time,
 * making this an "adaptive" algorithm.
 *
 * Notes:
 *
 *   1. This implementation supports only random-access iterators.
 *   2. The algorithm requires O(N) temporary storage.  The caller can
 *      override how to allocate this storage via the "copy" template
 *      parameter.
 */

template<typename Iter, typename Less, typename Copy>
void mergesort (Iter start, Iter end, Less less, Copy copy)
{
    typedef typename std::iterator_traits<Iter>::value_type Value;

    /* One step of an insertion sort: rotates the head item into place
     * within the sorted sub-list [head + 1, tail) */
    auto rotate_head = [less] (Iter head, Iter tail)
    {
        /* Find the proper location for the head item.  Skip *(head+1)
         * since we already know it is less than *(head). */
        Iter dest = head + 2;
        while (dest < tail && less (* dest, * head))
            dest ++;

        /* equivalent of std::rotate, inlined for speed */
        Value tmp = std::move (* head);
        std::move (head + 1, dest, head);
        * (dest - 1) = std::move (tmp);
    };

    /* Merges the two sorted sub-lists [head, mid) and [mid, tail) */
    auto do_merge = [less, copy] (Iter head, Iter mid, Iter tail)
    {
        /* copy list "a" to temporary storage */
        auto & buf = copy (head, mid);

        auto a = buf.begin ();
        auto a_end = a + (mid - head);
        Iter b = mid;
        Iter dest = head;

        /* the exit conditions of this loop are separated as an optimization */
        while (1)
        {
            if (! less (* b, * a))
            {
                * (dest ++) = std::move (* a);
                /* we already know b < tail, so don't waste time checking it */
                if ((++ a) == a_end)
                    break;
            }
            else
            {
                * (dest ++) = std::move (* b);
                /* we already know a < a_end, so don't waste time checking it */
                if ((++ b) == tail)
                    break;
            }
        }

        /* copy remainder of list "a" */
        std::move (a, a_end, dest);
    };

    /* A list with 0 or 1 element is sorted by definition. */
    if (end - start < 2)
        return;

    /* The algorithm runs right-to-left (so that insertions are left-to-right). */
    Iter head = end;

    /* Markers recording the divisions between sorted sub-lists or "runs".
     * Each run is at least 2x the length of its left-hand neighbor, so in
     * theory a list of 2^64 - 1 elements will have no more than 64 runs. */
    Iter div[64];
    int n_div = 0;

    do
    {
        Iter mid = head;
        head --;

        /* Scan right-to-left to find a run of increasing values.
         * If necessary, use insertion sort to create a run at 4 values long.
         * At this scale, insertion sort is faster due to lower overhead. */
        while (head > start)
        {
            if (less (* head, * (head - 1)))
            {
                if (mid - head < 4)
                    rotate_head (head - 1, mid);
                else
                    break;
            }

            head --;
        }

        /* Merge/collapse sub-lists left-to-right to maintain the invariant. */
        while (n_div >= 1)
        {
            Iter tail = div[n_div - 1];

            while (n_div >= 2)
            {
                Iter tail2 = div[n_div - 2];

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

                do_merge (mid, tail, tail2);

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

            if (head > start && (mid - head) <= (tail - mid) / 2)
                break;

            do_merge (head, mid, tail);

            mid = tail;
            n_div --;
        }

        /* push the new sub-list onto the stack */
        div[n_div] = mid;
        n_div ++;
    }
    while (head > start);
}

template<typename Iter, typename Less>
void mergesort (Iter start, Iter end, Less less)
{
    typedef typename std::iterator_traits<Iter>::value_type Value;

    /* Temporary storage for the algorithm */
    std::vector<Value> buf;

    auto copy_to_buf = [& buf] (Iter start, Iter end) -> std::vector<Value> &
    {
        /* Move items directly onto the existing vector if it's big enough.
         * Otherwise, create a new one; this is significantly faster than
         * appending using std::back_inserter.  Note: end() - begin() is
         * equivalent to size() but avoids a signed/unsigned comparison
         * warning. */
        if (end - start > buf.end () - buf.begin ())
            buf = std::vector<Value> (std::make_move_iterator (start),
                                      std::make_move_iterator (end));
        else
            std::move (start, end, buf.begin ());

        return buf;
    };

    mergesort (start, end, less, copy_to_buf);
}

template<typename Iter>
void mergesort (Iter start, Iter end)
{
    typedef typename std::iterator_traits<Iter>::value_type Value;
    mergesort (start, end, std::less<Value> ());
}

#endif
