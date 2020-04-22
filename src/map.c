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

int isWall(map * map, int col, int row, int doors_are_walls)
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
        if (!doors_are_walls)
            return 0;
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

void calculateStartToKeyDistances(map * map)
{
    printf("calculating start-to-key distances for map %d\n", map);
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
                        if ((!isWall(map, col, row-1, 0)) && (steps[col][row-1]==NOT_WORKED))
                        {
                            workToDo=1;
                            steps[col][row-1]=nextLevel;
                        }
                    }
                    // down
                    if (row < map->max_row-1)
                    {
                        if ((!isWall(map, col, row+1, 0)) && (steps[col][row+1]==NOT_WORKED))
                        {
                            workToDo=1;
                            steps[col][row+1]=nextLevel;
                        }
                    }
                    // left
                    if (col > 0)
                    {
                        if ((!isWall(map, col-1, row, 0)) && (steps[col-1][row]==NOT_WORKED))
                        {
                            workToDo=1;
                            steps[col-1][row]=nextLevel;
                        }
                    }
                    // right
                    if (col < map->max_col-1)
                    {
                        if ((!isWall(map, col+1, row, 0)) && (steps[col+1][row]==NOT_WORKED))
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
        if (map->keys[i]!=DOES_NOT_EXIST)
        {
            if(steps[map->key_location[i].col][map->key_location[i].row]!=NOT_WORKED)
            {
                printf("There are %d steps from the start to key %c\n", steps[map->key_location[i].col][map->key_location[i].row], i+MIN_KEY);
                map->steps_from_start_to_key[i]=steps[map->key_location[i].col][map->key_location[i].row];
            }
            else
            {
                printf("Cannot reach key %c from the start\n", i+MIN_KEY);
                map->steps_from_start_to_key[i]=NOT_WORKED;
            }
            
            for (int j=0; j<MAX_KEYS; j++)
            {
                map->doors_blocking_keys[i][j]=KEY_NOT_REQUIRED;
            }
            
            int current_step=steps[map->key_location[i].col][map->key_location[i].row];
            point current_location=map->key_location[i];

            while (current_step > 0)
            {
                current_step--;
                // up
                if ((current_location.row > 0) && (steps[current_location.col][current_location.row-1]==current_step))
                {
                    current_location.row--;
                }
                // down
                else if ((current_location.row < map->max_row-1) && (steps[current_location.col][current_location.row+1]==current_step))
                {
                    current_location.row++;
                }
                // left
                else if ((current_location.col > 0) && (steps[current_location.col-1][current_location.row]==current_step))
                {
                    current_location.col--;
                }
                // right
                else if ((current_location.col < map->max_col-1) && (steps[current_location.col+1][current_location.row]==current_step))
                {
                    current_location.col++;
                }
                else
                {
                    printf("Messed something else\n");
                }
                
                if (isDoor(map->layout[current_location.col][current_location.row]))
                {
                    map->doors_blocking_keys[i][map->layout[current_location.col][current_location.row]-MIN_DOOR]=KEY_REQUIRED;
                    printf("Key %c requires door %c to be unlocked.\n", i+MIN_KEY, map->layout[current_location.col][current_location.row]);
                }
            }
            
        }
        
    }
}

void calculateKeyToKeyDistances(map * map)
{
    printf("calculating key-to-key distances for map %d\n", map);
    printf("key values are :\n");
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (map->keys[i]!=DOES_NOT_EXIST)
        {
            printf(" working on distances from key %c - %d at col=%d row=%d\n", i+MIN_KEY, map->keys[i], map->key_location[i].col, map->key_location[i].row);
            int steps[MAX_MAP_DIMENSION][MAX_MAP_DIMENSION];
            for (int col=0; col<map->max_col; col++)
            {
                for (int row=0; row<map->max_row; row++)
                {
                    steps[col][row]=NOT_WORKED;
                }
            }
            
            point location=map->key_location[i];
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
                                if ((!isWall(map, col, row-1, 0)) && (steps[col][row-1]==NOT_WORKED))
                                {
                                    workToDo=1;
                                    steps[col][row-1]=nextLevel;
                                }
                            }
                            // down
                            if (row < map->max_row-1)
                            {
                                if ((!isWall(map, col, row+1, 0)) && (steps[col][row+1]==NOT_WORKED))
                                {
                                    workToDo=1;
                                    steps[col][row+1]=nextLevel;
                                }
                            }
                            // left
                            if (col > 0)
                            {
                                if ((!isWall(map, col-1, row, 0)) && (steps[col-1][row]==NOT_WORKED))
                                {
                                    workToDo=1;
                                    steps[col-1][row]=nextLevel;
                                }
                            }
                            // right
                            if (col < map->max_col-1)
                            {
                                if ((!isWall(map, col+1, row, 0)) && (steps[col+1][row]==NOT_WORKED))
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
            for (int j=0; j<MAX_KEYS; j++)
            {
                if (map->keys[j]!=DOES_NOT_EXIST)
                {
                    if (steps[map->key_location[i].col][map->key_location[i].row]!=NOT_WORKED)
                    {
                        printf("There are %d steps from key %c (%d) to key %c (%d)\n", steps[map->key_location[j].col][map->key_location[j].row], i+MIN_KEY, i, j+MIN_KEY, j);
                        map->steps_from_key_to_key[i][j]=steps[map->key_location[j].col][map->key_location[j].row];
                    }
                    else
                    {
                        printf("Cannot reach key %c from key %c\n", j+MIN_KEY, i+MIN_KEY);
                        map->steps_from_key_to_key[i][j]=NOT_WORKED;
                    }
                }
            }
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
    newMap->best_child_map_steps=999999999;
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
    startMap->best_child_map_steps=999999999;
}

void makeChildrenMaps(map * parentMap)
{
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (parentMap->steps_to_key[i]!=NOT_WORKED && parentMap->keys[i]==KEY_NOT_OBTAINED)
        {
//            printf("Setting map for key %c\n", i+MIN_KEY);
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
    //printf("Working the following map at level %d\n", level);
    //print_map(parentMap);
    if (allKeysObtained(parentMap))
    {
        //printf("All keys obtained\n");
        return;
    }

    calculateStartToKeyDistances(parentMap);
    calculateKeyToKeyDistances(parentMap);
    
    int best_steps=recusrive_work_it(parentMap, NULL, 0, 0, 999999999);
    printf("best_steps after recursion is %d\n", best_steps);
    //makeChildrenMaps(parentMap);
        
    parentMap->best_child_map_steps=best_steps;
    //deleteChildrenMaps(parentMap);
}

// current_path is list of keys
int recusrive_work_it(map * map, int * current_path, int current_path_len, int current_path_steps, int best_path_steps)
{
    printf("best path steps is %d\n", best_path_steps);
    printf("current path length is %d - ", current_path_len);
    int avail_steps[MAX_KEYS];
    int next_path[MAX_KEYS];
    for (int i=0; i<current_path_len; i++)
    {
        printf("%c", current_path[i]);
        avail_steps[current_path[i]-MIN_KEY]=VISITED;
        next_path[i]=current_path[i];
    }
    printf("\n");
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (avail_steps[i]==VISITED)
            continue;
        
        if (map->keys[i]==DOES_NOT_EXIST)
            avail_steps[i]=DOES_NOT_EXIST;
        else
        {
            avail_steps[i]=AVAILABLE;
            for (int j=0; j<MAX_KEYS; j++)
            {
                if (map->doors_blocking_keys[i][j]==KEY_REQUIRED && avail_steps[j]!=VISITED)
                {
                    printf("key %c is not available because key %c has not been visited  door_blocking_keys[i][j]=%d avail-steps[j]=%d\n", i+MIN_KEY, j+MIN_KEY, map->doors_blocking_keys[i][j], avail_steps[j]);
                    avail_steps[i]=NOT_AVAILABLE;
                    break;
                }
            }
        }
    }

    char current_position='@';
    if (current_path_len>0)
        current_position=current_path[current_path_len-1];
    
    int next_path_len=current_path_len+1;
    
    int has_avail_steps=0;
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (avail_steps[i]==AVAILABLE)
        {
            has_avail_steps=1;
            next_path[current_path_len]=i+MIN_KEY;
            printf("Checking path ");
            for (int j=0; j<next_path_len; j++)
            {
                printf("%c", next_path[j]);
            }
            printf("\n");
            int next_path_steps=current_path_steps;
            if (next_path_len==1) // moving from start
            {
                next_path_steps+=map->steps_from_start_to_key[i];
                printf("Adding %d steps from start to %c to get new total of %d\n", map->steps_from_start_to_key[i], i+MIN_KEY, next_path_steps);
            }
            else
            {
                printf("current position is %d, i is %d with steps %d\n", current_position-MIN_KEY, i, map->steps_from_key_to_key[current_position-MIN_KEY][i]);
                next_path_steps+=map->steps_from_key_to_key[current_position-MIN_KEY][i];
                printf("Adding %d steps from %c to %c to get new total of %d\n", map->steps_from_key_to_key[current_position-MIN_KEY][i], current_position, i+MIN_KEY, next_path_steps);
            }
            if (next_path_steps >= best_path_steps)
            {
                printf("   skipping from here; next path steps %d is greater than best path steps of %d\n", next_path_steps, best_path_steps);
                continue;
            }
            best_path_steps = recusrive_work_it(map, next_path, next_path_len, next_path_steps, best_path_steps);
            printf("after recursion call, best_path_steps is %d\n", best_path_steps);
        }
    }
    
    if (!has_avail_steps)
        best_path_steps=current_path_steps;
    
    return best_path_steps;
}

void deleteChildrenMaps(map * parentMap)
{
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (parentMap->child_by_key[i]!=NULL)
        {
            //deleteChildrenMaps(parentMap->child_by_key[i]);
            free(parentMap->child_by_key[i]);
            parentMap->child_by_key[i]=NULL;
        }
    }
}

int findBestMapSteps(map * parentMap)
{
    int bestSteps=999999999;
    for (int i=0; i<MAX_KEYS; i++)
    {
        map * childMap = parentMap->child_by_key[i];
        if (childMap != NULL)
        {
            if (allKeysObtained(childMap))
                childMap->best_child_map_steps = childMap->steps_from_start;
            if (childMap->best_child_map_steps < bestSteps)
                bestSteps=childMap->best_child_map_steps;
        }
    }
    return bestSteps;
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
