#include <stdio.h>
#include <stdlib.h>

#include "map.h"

int isKey(char ch)
{
    return (ch >= MIN_KEY && ch <= MAX_KEY);
}

int isDoor(char ch)
{
    return (ch >= MIN_DOOR && ch <= MAX_DOOR);
}    

int hasKey(map * map, char door)
{
    return (map->keys[door-MIN_DOOR]==KEY_OBTAINED ? 1 : 0);
}

int isWall(map * map, int col, int row)
{
    if (col < 0 || col >= map->max_col || row < 0 || row >= map->max_row)
    {
        fprintf(stderr, "Error - attempting to check for wall at (%d,%d) while map goes from (0,0) to (%d,%d)\n", col, row, map->max_col, map->max_row);
        exit(-1);
    }
    char val=map->layout[col][row];
    if (val==WALL)
        return 1;
    if (val==SPACE || val==ME)
        return 0;
    if (isKey(val)) // keys are never walls
        return 0;
    if (isDoor(val))
    {
        // doors are walls when we don't have the keys
        return !hasKey(map, val);
    }
    fprintf(stderr, "should not get here - unknown value %c(%d) for isWall\n", val, val);
    exit(-1);
}

int allKeysObtained(map * map)
{
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (map->keys[i]==KEY_NOT_OBTAINED)
            return 0;
    }
    return 1;
}

void calculateKeyDistances(map * map)
{
    printf("calculating key distances for map %d\n", map);
    printf("key values are :\n");
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (map->keys[i]!=DOES_NOT_EXIST)
        {
            printf("  %c - %d at col=%d row=%d\n", i+MIN_KEY, map->keys[i], map->key_location[i].col, map->key_location[i].row);
        }
    }
    int steps[MAX_MAP_DIMENSION][MAX_MAP_DIMENSION];
    for (int col=0; col<map->max_col; col++)
    {
        for (int row=0; row<map->max_row; row++)
        {
            steps[col][row]=NOT_WORKED;
        }
    }
        
    point location=map->current_location;
    steps[location.col][location.row]=0;
    int level = 0;
    int workToDo = 1;
    while(workToDo != 0)
    {
        int nextLevel=level+1;
        workToDo=0;
        for (int col=0; col<map->max_col; col++)
        {
            for (int row=0; row<map->max_row; row++)
            {
                if (steps[col][row]==level)
                {
                    // up
                    if (row > 0)
                    {
                        if ((!isWall(map, col, row-1)) && (steps[col][row-1]==NOT_WORKED))
                        {
                            workToDo=1;
                            steps[col][row-1]=nextLevel;
                        }
                    }
                    // down
                    if (row < map->max_row-1)
                    {
                        if ((!isWall(map, col, row+1)) && (steps[col][row+1]==NOT_WORKED))
                        {
                            workToDo=1;
                            steps[col][row+1]=nextLevel;
                        }
                    }
                    // left
                    if (col > 0)
                    {
                        if ((!isWall(map, col-1, row)) && (steps[col-1][row]==NOT_WORKED))
                        {
                            workToDo=1;
                            steps[col-1][row]=nextLevel;
                        }
                    }
                    // right
                    if (col < map->max_col-1)
                    {
                        if ((!isWall(map, col+1, row)) && (steps[col+1][row]==NOT_WORKED))
                        {
                            workToDo=1;
                            steps[col+1][row]=nextLevel;
                        }
                    }
                }
            }
        }
        level=nextLevel;
    }
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (map->keys[i]==KEY_NOT_OBTAINED && steps[map->key_location[i].col][map->key_location[i].row]!=NOT_WORKED)
        {
            printf("There are %d steps to key %c\n", steps[map->key_location[i].col][map->key_location[i].row], i+MIN_KEY);
            map->steps_to_key[i]=steps[map->key_location[i].col][map->key_location[i].row];
        }
        else
        {
            map->steps_to_key[i]=NOT_WORKED;
        }
    }
}

map * dupeForChildMap(map * parentMap)
{
    map * newMap = (map *)malloc(sizeof(map));
    newMap->max_row=parentMap->max_row;
    newMap->max_col=parentMap->max_col;
    for (int col=0; col<parentMap->max_col; col++)
    {
        for (int row=0; row<parentMap->max_row; row++)
        {
            newMap->layout[col][row]=parentMap->layout[col][row];
        }
    }
    
    for (int i=0; i<MAX_KEYS; i++)
    {
        newMap->doors[i]=parentMap->doors[i];
        newMap->door_location[i].col=parentMap->door_location[i].col;
        newMap->door_location[i].row=parentMap->door_location[i].row;
        newMap->keys[i]=parentMap->keys[i];
        newMap->key_location[i].col=parentMap->key_location[i].col;
        newMap->key_location[i].row=parentMap->key_location[i].row;
        newMap->child_by_key[i]=NULL;
    }
    newMap->parent=parentMap;
}

void initStartMap(map * startMap)
{
    startMap->max_row=0;
    startMap->max_col=0;
    for (int i=0; i<MAX_KEYS; i++)
    {
        startMap->doors[i]=DOES_NOT_EXIST;
        startMap->keys[i]=DOES_NOT_EXIST;
        startMap->child_by_key[i]=NULL;
    }
    startMap->parent=NULL;
    startMap->steps_from_parent=0;
    startMap->steps_from_start=0;
}

void makeChildrenMaps(map * parentMap)
{
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (parentMap->steps_to_key[i]!=NOT_WORKED && parentMap->keys[i]==KEY_NOT_OBTAINED)
        {
            printf("Setting map for key %c\n", i+MIN_KEY);
            map * newMap = dupeForChildMap(parentMap);
            parentMap->child_by_key[i]=newMap;
            newMap->steps_from_parent=parentMap->steps_to_key[i];
            newMap->steps_from_parent=parentMap->steps_to_key[i];
            newMap->steps_from_start=parentMap->steps_from_start+parentMap->steps_to_key[i];
            newMap->keys[i]=KEY_OBTAINED;
            newMap->layout[parentMap->current_location.col][parentMap->current_location.row]=SPACE;
            if (parentMap->doors[i]==DOOR_EXISTS)
                newMap->layout[parentMap->door_location[i].col][parentMap->door_location[i].row]=SPACE;
            newMap->current_location=parentMap->key_location[i];
            newMap->layout[newMap->current_location.col][newMap->current_location.row]=ME;
            //printf("Set map is :\n");
            //print_map(newMap);
        }
    }
}

void buildAndWorkChildrenMaps(map * parentMap, int level)
{
    printf("Working the following map at level %d\n", level);
    print_map(parentMap);
    if (allKeysObtained(parentMap))
    {
        printf("All keys obtained\n");
        return;
    }

    calculateKeyDistances(parentMap);
    makeChildrenMaps(parentMap);
        
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (parentMap->child_by_key[i]!=NULL)
        {
            for (int i=0; i<level; i++)
                printf("%c", ' ');
            printf("%c\n", i+MIN_KEY);
            buildAndWorkChildrenMaps(parentMap->child_by_key[i], level+1);
        }
    }
}

void deleteChildrenMaps(map * parentMap)
{
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (parentMap->child_by_key[i]!=NULL)
        {
            deleteChildrenMaps(parentMap->child_by_key[i]);
            free(parentMap->child_by_key[i]);
        }
    }
}

map * findBestMap(map * parentMap)
{
    print_map(parentMap);
    map * bestMap=NULL;
    for (int i=0; i<MAX_KEYS; i++)
    {
        map * childMap = parentMap->child_by_key[i];
        if (childMap != NULL)
        {
            //print_map(childMap);
            if (allKeysObtained(childMap))
            {
                if (bestMap == NULL || (childMap->steps_from_start < bestMap->steps_from_start))
                    bestMap = childMap;
            }
            else
            {
                map * bestChildMap = findBestMap(childMap);
                if (bestMap == NULL || (bestChildMap->steps_from_start < bestMap->steps_from_start))
                    bestMap = bestChildMap;
            }
        }
    }
    return bestMap;
}

void print_map(map * theMap)
{
    printf("Map has %d rows and %d cols\n", theMap->max_row, theMap->max_col);
    for (int row=0; row<theMap->max_row; row++)
    {
        for (int col=0; col<theMap->max_col; col++)
        {
            printf("%c", theMap->layout[col][row]);
        }
        printf("\n");
    }
}
