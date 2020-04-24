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


void init_multi_cache(multi_cache * c)
{
    c->nodes_by_position_num_following=NULL;
    c->full_case_node=NULL;
}

void delete_multi_cache(multi_cache * c)
{
    if (c->nodes_by_position_num_following==NULL)
        return;
    for (int i=0; i<=MAX_KEYS; i++)
    {
        multi_cache_node ***** iNodePtr=c->nodes_by_position_num_following[i];
        if (iNodePtr==NULL)
            continue;
        
        for (int j=0; j<=MAX_KEYS; j++)
        {
            multi_cache_node **** jNodePtr=iNodePtr[j];
            if (jNodePtr==NULL)
                continue;
            
            for (int k=0; k<=MAX_KEYS; k++)
            {
                multi_cache_node *** kNodePtr=jNodePtr[k];
                if (kNodePtr==NULL)
                    continue;
                
                for (int l=0; l<=MAX_KEYS; l++)
                {
                    multi_cache_node ** lNodePtr=kNodePtr[l];
                    if (lNodePtr==NULL)
                        continue;
                    
                    for (int m=0; m<MAX_KEYS; m++)
                    {
                        multi_cache_node * current=lNodePtr[m];
                        while (current != NULL)
                        {
                            multi_cache_node * tmp=current->next;
                            free(current);
                            current=tmp;
                        }
                    }
                    free (lNodePtr);
                }
                free (kNodePtr);
            }
            free (jNodePtr);
        }
        free (iNodePtr);
    }
    if (c->full_case_node != NULL)
        free(c->full_case_node);
}

void insert_multi_cache(multi_cache * c, int * current_position, int * keys_to_get, int best_steps)
{
    int key_count=0;
    multi_cache_node * n = (multi_cache_node *)malloc(sizeof(multi_cache_node));
    int is_full_case=1;
    for (int i=0; i<NUM_SUBMAPS; i++)
    {
        n->current_position[i]=current_position[i];
        if (current_position[i]!='@')
            is_full_case=0;
    }
    
    for (int i=0; i<MAX_KEYS; i++)
    {
        n->keys_to_get[i]=keys_to_get[i];
        if (keys_to_get[i]==KEY_NOT_OBTAINED)
            key_count++;
    }
    n->best_steps=best_steps;
    n->next=NULL;
    
    if (is_full_case==1)
        c->full_case_node=n;
    else
    {
        int indexes[NUM_SUBMAPS];
        for (int i=0; i<NUM_SUBMAPS; i++)
            indexes[i]=current_position[i]=='@'?MAX_KEYS:current_position[i]-MIN_KEY; // send to 26 if @ or the index for any other

        if (c->nodes_by_position_num_following==NULL)
        {
            c->nodes_by_position_num_following=(multi_cache_node ******)malloc((MAX_KEYS+1)*sizeof(multi_cache_node *****));
            for (int i=0; i<=MAX_KEYS; i++)
                c->nodes_by_position_num_following[i]=NULL;
        }
        
        multi_cache_node ***** node0=c->nodes_by_position_num_following[indexes[0]]; // 0
        if (node0==NULL)
        {
            c->nodes_by_position_num_following[indexes[0]]=(multi_cache_node *****)malloc((MAX_KEYS+1)*sizeof(multi_cache_node ****));
            node0=c->nodes_by_position_num_following[indexes[0]];
            for (int i=0; i<=MAX_KEYS; i++)
                node0[i]=NULL;
        }
        
        multi_cache_node **** node1=node0[indexes[1]];
        if (node1==NULL)
        {
            node0[indexes[1]]=(multi_cache_node ****)malloc((MAX_KEYS+1)*sizeof(multi_cache_node ***));
            node1=node0[indexes[1]];
            for (int i=0; i<=MAX_KEYS; i++)
                node1[i]=NULL;
        }
        
        multi_cache_node *** node2=node1[indexes[2]];
        if (node2==NULL)
        {
            node1[indexes[2]]=(multi_cache_node ***)malloc((MAX_KEYS+1)*sizeof(multi_cache_node **));
            node2=node1[indexes[2]];
            for (int i=0; i<=MAX_KEYS; i++)
                node2[i]=NULL;
        }
        
        multi_cache_node ** node3=node2[indexes[3]];
        if (node3==NULL)
        {
            node2[indexes[3]]=(multi_cache_node **)malloc((MAX_KEYS)*sizeof(multi_cache_node *));
            node3=node2[indexes[3]];
            for (int i=0; i<MAX_KEYS; i++)
                node3[i]=NULL;
        }
                        
        multi_cache_node * current=node3[key_count];
        if (current==NULL)
        {
            node3[key_count]=n;
        }
        else
        {
            while (current->next != NULL)
                current=current->next;
            current->next=n;
        }
    }
}

multi_cache_node * find_multi_cache(multi_cache * c, int * current_position, int * keys_to_get)
{
    int indexes[NUM_SUBMAPS];
    int is_full_case=1;
    for (int i=0; i<NUM_SUBMAPS; i++)
    {
        if (current_position[i]!='@')
            is_full_case=0;
        indexes[i]=current_position[i]=='@'?MAX_KEYS:current_position[i]-MIN_KEY; // send to 26 if @ or the index for any other
    }
    
    if (is_full_case)
        return c->full_case_node;
    
    int key_count=0;
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (keys_to_get[i]==KEY_NOT_OBTAINED)
            key_count++;
    }
    
    
    if (c->nodes_by_position_num_following==NULL)
        return NULL;
    
    multi_cache_node ***** node0=c->nodes_by_position_num_following[indexes[0]];
    if (node0 == NULL)
        return NULL;
        
    multi_cache_node **** node1=node0[indexes[1]];
    if (node1 == NULL)
        return NULL;
    
    multi_cache_node *** node2=node1[indexes[2]];
    if (node2 == NULL)
        return NULL;
    
    multi_cache_node ** node3=node2[indexes[3]];
    if (node3 == NULL)
        return NULL;
                        
    multi_cache_node * current=node3[key_count];
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
