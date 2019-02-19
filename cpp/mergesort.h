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
 */

template<typename Iter, typename Less>
class MergeSort
{
private:
    typedef typename std::iterator_traits<Iter>::value_type Value;

    /* Inserts a single element into a sorted list */
    static void insert_head (Iter head, Iter tail, Less less)
    {
        Iter dest;

        for (dest = head + 1; dest + 1 < tail; dest ++)
        {
            if (! less (* (dest + 1), * head))
                break;
        }

        Value tmp = std::move (* head);
        std::move (head + 1, dest + 1, head);
        * dest = std::move (tmp);
    }

    /* Merges two sorted sub-lists */
    static void do_merge (Iter head, Iter mid, Iter tail, Less less,
                          std::vector<Value> & buf)
    {
        buf.clear ();
        buf.reserve (mid - head);

        /* copy list "a" to temporary storage */
        std::move (head, mid, std::back_inserter (buf));

        auto a = buf.begin ();
        auto a_end = buf.end ();
        Iter b = mid;
        Iter dest = head;

        /* Handle the case of strictly separate (but reversed) lists specially.
         * In this case, we simply shift list "b" to the left. */
        if (less (* (tail - 1), * head))
        {
            std::move (b, tail, dest);
            dest += tail - b;
            b = tail;
        }

        for (; a < a_end && b < tail; dest ++)
        {
            if (! less (* b, * a)) {
                * dest = std::move (* a);
                a ++;
            } else {
                * dest = std::move (* b);
                b ++;
            }
        }

        /* copy remainder of list "a" */
        std::move (a, a_end, dest);
    }

public:
    /* Top-level merge sort algorithm */
    static void sort (Iter start, Iter end, Less less)
    {
        /* A list with 0 or 1 element is sorted by definition. */
        if (end - start < 2)
            return;

        std::vector<Value> buf;

        /* The algorithm runs right-to-left (so that insertions are left-to-right). */
        Iter head = end;
        Iter mid, tail, tail2;

        /* Markers recording the divisions between sorted sub-lists or "runs".
         * Each run is at least 2x the length of its left-hand neighbor, so in
         * theory a list of 2^64 - 1 elements will have no more than 64 runs. */
        Iter div[64];
        int n_div = 0;

        do
        {
            mid = head;
            head = mid - 1;

            /* Scan right-to-left to find a run of increasing values.
             * If necessary, use insertion sort to create a run at 4 values long.
             * At this scale, insertion sort is faster due to lower overhead. */
            while (head > start)
            {
                if (less (* head, * (head - 1)))
                {
                    if (mid - head < 4)
                        insert_head (head - 1, mid, less);
                    else
                        break;
                }

                head --;
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

                    do_merge (mid, tail, tail2, less, buf);

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

                do_merge (head, mid, tail, less, buf);

                mid = tail;
                n_div --;
            }

            /* push the new sub-list onto the stack */
            div[n_div] = mid;
            n_div ++;
        }
        while (head > start);
    }
};

template<typename Iter, typename Less>
void mergesort (Iter start, Iter end, Less less)
    { MergeSort<Iter, Less>::sort (start, end, less); }

template<typename Iter>
void mergesort (Iter const start, Iter const end)
{
    typedef typename std::iterator_traits<Iter>::value_type Value;
    mergesort (start, end, std::less<Value> ());
}

#endif
