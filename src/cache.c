#include <stdio.h>
#include <stdlib.h>

#include "cache.h"
#include "map.h"

void init_cache(cache * c)
{
    for (int i=0; i<MAX_KEYS; i++)
    {
        for (int j=0; j<MAX_KEYS; j++)
        {
            c->nodes_by_position_num_following[i][j]=NULL;
        }
    }
}

void delete_cache(cache * c)
{
    for (int i=0; i<MAX_KEYS; i++)
    {
        for (int j=0; j<MAX_KEYS; j++)
        {
            cache_node * current=c->nodes_by_position_num_following[i][j];
            while (current != NULL)
            {
                cache_node * tmp=current->next;
                free(current);
                current=tmp;
            }
        }
    }
}

void insert(cache * c, int current_position, int * keys_to_get, int best_steps)
{
    int key_count=0;
    cache_node * n = (cache_node *)malloc(sizeof(cache_node));
    n->current_position=current_position;
    for (int i=0; i<MAX_KEYS; i++)
    {
        n->keys_to_get[i]=keys_to_get[i];
        if (keys_to_get[i]==KEY_NOT_OBTAINED)
            key_count++;
    }
    n->best_steps=best_steps;
    n->next=NULL;
    cache_node * current=c->nodes_by_position_num_following[current_position-MIN_KEY][key_count];
    if (current==NULL)
    {
        c->nodes_by_position_num_following[current_position-MIN_KEY][key_count]=n;
    }
    else
    {
        while (current->next != NULL)
            current=current->next;
        current->next=n;
    }
}

cache_node * find(cache * c, int current_position, int * keys_to_get)
{
    int key_count=0;
    cache_node * n = (cache_node *)malloc(sizeof(cache_node));
    n->current_position=current_position;
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (keys_to_get[i]==KEY_NOT_OBTAINED)
            key_count++;
    }
    cache_node * current=c->nodes_by_position_num_following[current_position-MIN_KEY][key_count];
    while (current != NULL)
    {
        int match=1;
        for (int i=0; i<MAX_KEYS; i++)
        {
            if ((keys_to_get[i]==KEY_NOT_OBTAINED && current->keys_to_get[i]!=KEY_NOT_OBTAINED) ||
                (keys_to_get[i]!=KEY_NOT_OBTAINED && current->keys_to_get[i]==KEY_NOT_OBTAINED))
            {
                match=0;
                break;
            }
        }
        if (match)
            return current;
        current=current->next;
    }
    return NULL;
}
