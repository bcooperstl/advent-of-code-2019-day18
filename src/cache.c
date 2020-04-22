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
    c->full_case_node=NULL;
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
    if (c->full_case_node != NULL)
        free(c->full_case_node);
}

void insert_cache(cache * c, int current_position, int * keys_to_get, int best_steps)
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
    
    if (current_position=='@')
        c->full_case_node=n;
    else
    {
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
}

cache_node * find_cache(cache * c, int current_position, int * keys_to_get)
{
    if (current_position=='@')
        return c->full_case_node;
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
