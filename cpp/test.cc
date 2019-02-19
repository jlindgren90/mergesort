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

/*
 * Test driver for the merge-sort algorithm
 */

#include "mergesort.h"
#include "timsort.h"

#include <glib.h>

struct Item
{
    int val;
    int idx;

    bool operator< (const Item & b) const
        { return val < b.val; }
};

std::vector<Item> gen_array (int n_items, int n_swaps, bool rev)
{
    std::vector<Item> items (n_items);

    /* start with a sorted array (forward or reverse) */
    if (rev) {
        for (int i = 0; i < n_items; i ++)
            items[i].val = n_items - 1 - i;
    } else {
        for (int i = 0; i < n_items; i ++)
            items[i].val = i;
    }

    /* introduce randomness by swapping pairs of items */
    for (int i = 0; i < n_swaps; i ++)
    {
        int a = g_random_int_range (0, n_items);
        int b = g_random_int_range (0, n_items);

        int temp = items[a].val;
        items[a].val = items[b].val;
        items[b].val = temp;
    }

    /* index items to check stability later */
    for (int i = 0; i < n_items; i ++)
        items[i].idx = i;

    return items;
}

/* verifies correct ordering as well as stability */
void verify_sorted (const std::vector<Item> & items)
{
    for (int i = 0; i < (int) items.size () - 1; i ++)
    {
        if (items[i].val > items[i + 1].val ||
              (items[i].val == items[i + 1].val &&
               items[i].idx > items[i + 1].idx))
            abort ();
    }
}

/* broken out for profiling */
void stdsort (std::vector<Item> & items) __attribute__ ((noinline));
void timsort (std::vector<Item> & items) __attribute__ ((noinline));
void mergesort (std::vector<Item> & items) __attribute__ ((noinline));

void stdsort (std::vector<Item> & items)
    { std::stable_sort (std::begin (items), std::end (items)); }

void timsort (std::vector<Item> & items)
    { gfx::timsort (std::begin (items), std::end (items)); }

void mergesort (std::vector<Item> & items)
    { mergesort (std::begin (items), std::end (items)); }

int main (void)
{
    g_random_set_seed (0);

    for (int n_items = 1; n_items < 65536; n_items *= 2)
    {
        for (int n_swaps = 1; n_swaps < n_items; n_swaps *= 2)
        {
            std::vector<Item> items;

            items = gen_array (n_items, n_swaps, false);
            stdsort (items);
            verify_sorted (items);

            items = gen_array (n_items, n_swaps, true);
            stdsort (items);
            verify_sorted (items);

            items = gen_array (n_items, n_swaps, false);
            timsort (items);
            verify_sorted (items);

            items = gen_array (n_items, n_swaps, true);
            timsort (items);
            verify_sorted (items);

            items = gen_array (n_items, n_swaps, false);
            mergesort (items);
            verify_sorted (items);

            items = gen_array (n_items, n_swaps, true);
            mergesort (items);
            verify_sorted (items);
        }
    }

    return 0;
}
