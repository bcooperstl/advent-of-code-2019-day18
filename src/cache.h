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
    cache_node * full_case_node;
    cache_node * nodes_by_position_num_following[MAX_KEYS][MAX_KEYS]; // first index is position. second index is the number of keys in the steps.
};

typedef struct cache cache;

void init_cache(cache * cache);
void delete_cache(cache * cache);
void insert_cache(cache * c, int current_position, int * keys_to_get, int best_steps);
cache_node * find_cache(cache * c, int current_position, int * keys_to_get);

struct multi_cache_node
{
    int current_position[NUM_SUBMAPS];
    int keys_to_get[MAX_KEYS];
    int best_steps;
    struct multi_cache_node * next;
};

typedef struct multi_cache_node multi_cache_node;

struct multi_cache
{
    multi_cache_node * full_case_node;
    multi_cache_node ****** nodes_by_position_num_following; // first four indices are four position. fifth index is the number of keys in the steps.
};

typedef struct multi_cache multi_cache;

void init_multi_cache(multi_cache * multi_cache);
void delete_multi_cache(multi_cache * multi_cache);
void insert_multi_cache(multi_cache * c, int * current_position, int * keys_to_get, int best_steps);
multi_cache_node * find_multi_cache(multi_cache * c, int * current_position, int * keys_to_get);


#endif
