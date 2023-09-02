#include "bitmap.h"
#include "assert.h"
#include <stdlib.h>
#include <stdio.h>

void bitmap_init(struct bitmap *bitmap_p, int map_len)
{
    assert(map_len > 0);
    int bitmap_array_size = map_len % sizeof(char) == 0 ? map_len / sizeof(char) : map_len / sizeof(char) + 1;
    bitmap_p->bitmap_len = map_len;
    bitmap_p->bitmap_array = (char *)malloc(sizeof(char) * bitmap_array_size);
    for (int i = 0; i < bitmap_array_size; i++)
    {
        bitmap_p->bitmap_array[i] = 0;
    }
}

void btimap_set_one(struct bitmap *bitmap_p, int location)
{
    int bitmap_array_loc = location / sizeof(char);
    int bitmap_bit_loc = location % sizeof(char);
    bitmap_p->bitmap_array[bitmap_array_loc] |= 1UL << bitmap_bit_loc;
}

void btimap_set_zero(struct bitmap *bitmap_p, int location)
{
    int bitmap_array_loc = location / sizeof(char);
    int bitmap_bit_loc = location % sizeof(char);
    bitmap_p->bitmap_array[bitmap_array_loc] &= ~(1UL << bitmap_bit_loc);
}

int bitmap_bit_check(struct bitmap *bitmap_p, int location)
{
    int bitmap_array_loc = location / sizeof(char);
    int bitmap_bit_loc = location % sizeof(char);
    int check_value = (bitmap_p->bitmap_array[bitmap_array_loc] >> bitmap_bit_loc);
    return check_value;
}

void bitmap_destory(struct bitmap *bitmap_p)
{
    free(bitmap_p->bitmap_array);
}
