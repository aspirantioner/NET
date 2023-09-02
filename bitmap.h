#ifndef _BIT_MAP_H
#define _BIT_MAP_H

typedef struct bitmap
{
    char *bitmap_array;
    int bitmap_len;
} bitmap;

void bitmap_init(struct bitmap *bitmap_p, int map_len);
void btimap_set_one(struct bitmap *bitmap_p, int location);
void btimap_set_zero(struct bitmap *bitmap_p, int location);
void bitmap_destory(struct bitmap *bitmap_p);
int bitmap_bit_check(struct bitmap *bitmap_p, int location);
#endif