#ifndef __CACHE_H__
#define __CACHE_H__

#include "map.h"

struct cache_node
{
    int current_position;
    int keys_to_get[MAX_KEYS];
    int best_steps;
    struct cache_node * next;
};

typedef struct cache_node cache_node;

struct cache
{
    cache_node * nodes_by_position_num_following[MAX_KEYS][MAX_KEYS]; // first index is position. second index is the number of keys in the steps.
};

typedef struct cache cache;

void init_cache(cache * cache);
void delete_cache(cache * cache);

#endif
