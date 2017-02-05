#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

typedef struct {
    int val;
    int idx;
} Item;

/* mergesort.c */
void mergesort (void * items, int n_items, int size,
                GCompareDataFunc compare, void * data);

Item * gen_array (int n_items, int n_swaps, bool rev)
{
    Item * items = g_new (Item, n_items);

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

void print_array (const Item * items, int n_items)
{
    for (int i = 0; i < n_items; i ++)
        printf ("%d=%d ", items[i].idx, items[i].val);

    printf ("\n");
}

int compare_items (const void * a_, const void * b_, void * data)
{
    const Item * a = a_;
    const Item * b = b_;

    if (a->val < b->val)
        return -1;
    else if (a->val == b->val)
        return 0;
    else
        return 1;
}

void verify_sorted (const Item * items, int n_items)
{
    for (int i = 0; i < n_items - 1; i ++)
    {
        if (items[i].val > items[i + 1].val ||
              (items[i].val == items[i + 1].val &&
               items[i].idx > items[i + 1].idx))
            abort ();
    }
}

int main (void)
{
    for (int n_items = 1; n_items < 1024*1024; n_items *= 2)
    {
        for (int n_swaps = 1; n_swaps < n_items; n_swaps *= 2)
        {
            Item * items;

            items = gen_array (n_items, n_swaps, false);
            g_qsort_with_data (items, n_items, sizeof (Item), compare_items, NULL);
            verify_sorted (items, n_items);
            g_free (items);

            items = gen_array (n_items, n_swaps, true);
            g_qsort_with_data (items, n_items, sizeof (Item), compare_items, NULL);
            verify_sorted (items, n_items);
            g_free (items);

            items = gen_array (n_items, n_swaps, false);
            mergesort (items, n_items, sizeof (Item), compare_items, NULL);
            verify_sorted (items, n_items);
            g_free (items);

            items = gen_array (n_items, n_swaps, true);
            mergesort (items, n_items, sizeof (Item), compare_items, NULL);
            verify_sorted (items, n_items);
            g_free (items);
        }
    }

    return 0;
}
