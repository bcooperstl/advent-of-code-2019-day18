#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "cache.h"

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
                    printf("Key %c requires door %c to be unlocked. Setting doors_blocking_keys[%d][%d] to KEY_REQUIRED(%d)\n", i+MIN_KEY, map->layout[current_location.col][current_location.row], i, map->layout[current_location.col][current_location.row]-MIN_DOOR, KEY_REQUIRED);
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

void buildAndWorkChildrenMaps(map * parentMap, int level, cache * myCache)
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
    
    int best_steps=recursive_build_cache(parentMap, myCache, NULL, 0);
    printf("best_steps after recursion is %d\n", best_steps);
    //makeChildrenMaps(parentMap);
        
    parentMap->best_child_map_steps=best_steps;
    //deleteChildrenMaps(parentMap);
}

int build_keys_to_get(map * parentMap, int * current_path, int current_path_len, int * keys_to_get)
{
    //printf("building keys_to_get\n");
    int num_remaining=0;
    for (int i=0; i<MAX_KEYS; i++)
    {
        keys_to_get[i]=parentMap->keys[i];
        if (keys_to_get[i]!=DOES_NOT_EXIST)
        {
            //printf("incrementing for %c\n", i+MIN_KEY);
            num_remaining++;
        }
    }
    for (int i=0; i<current_path_len; i++)
    {
        //printf("decrementing for %c in current path position %d\n", current_path[i], i);
        keys_to_get[current_path[i]-MIN_KEY]=KEY_OBTAINED;
        num_remaining--;
    }
    //printf("returning %d keys to get\n", num_remaining);
    return num_remaining;
}

int recursive_build_cache(map * parentMap, cache * myCache, int * current_path, int current_path_len)
{
    int keys_to_get[26];
    int num_keys_to_get=build_keys_to_get(parentMap, current_path, current_path_len, keys_to_get);
    int current_position=(current_path_len==0?'@':current_path[current_path_len-1]);
    //printf("Current path is :");
    //for (int i=0; i<current_path_len; i++)
    //    printf("%c", current_path[i]);
    //printf("\n");
    //printf("There are %d keys to get at position %c\n", num_keys_to_get, current_position);
    if (num_keys_to_get==1) // special case - 1 key to get. just insert the distance from the current position to it.
    {
        for (int i=0; i<MAX_KEYS; i++)
        {
            if (keys_to_get[i]==KEY_NOT_OBTAINED)
                return parentMap->steps_from_key_to_key[current_position-MIN_KEY][i];
        }
    }
    // if here, num_keys_to_get is 2 or greater
    //    first check if there is a cache hit
    cache_node * hit = find_cache(myCache, current_position, keys_to_get);
    if (hit != NULL)
        return hit->best_steps;
    // if here, we have a cache miss. Need to recusrively go in, one option at a time.
    int next_path[MAX_KEYS];
    int next_path_len;
    int lowest_steps=999999999;
    
    for (int i=0; i<current_path_len; i++)
        next_path[i]=current_path[i];
    next_path_len=current_path_len+1;
    
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (keys_to_get[i]!=KEY_NOT_OBTAINED)
            continue;
        //printf("keys_to_get[%c] is KEY_NOT_OBTAINED\n", i+MIN_KEY);

        int not_required_key=0;
        for (int j=0; j<MAX_KEYS; j++)
        {
            //if (parentMap->doors_blocking_keys[i][j]==KEY_REQUIRED)
            //{
            //    printf("key %c is required before key %c\n", j+MIN_KEY, i+MIN_KEY);
            //    printf("keys_to_get[%d] is %d. KEY_OBTAINED is %d\n", j, keys_to_get[j], KEY_OBTAINED);
            //    //printf(doors_blocking_keys);
            //}
            
            if (parentMap->doors_blocking_keys[i][j]==KEY_REQUIRED && keys_to_get[j]!=KEY_OBTAINED)
            {
            //    printf("key %c is not available because key %c has not been visited\n", i+MIN_KEY, j+MIN_KEY);
                not_required_key=1;
                break;
            }
        }
        if (not_required_key==1)
            continue;

        // set up the next step
        int steps_current_to_next=0;
        if (current_path_len==0)
            steps_current_to_next=parentMap->steps_from_start_to_key[i]; // the steps from start to i
        else 
            steps_current_to_next=parentMap->steps_from_key_to_key[current_position-MIN_KEY][i]; // the steps from the last position to get to i
        keys_to_get[i]=KEY_OBTAINED;
        next_path[next_path_len-1]=i+MIN_KEY;
        //printf("Calling recusrively from path: ");
        //for (int i=0; i<current_path_len; i++)
        //    printf("%c", current_path[i]);
        //printf("\n");
        
        int steps_next_to_end=recursive_build_cache(parentMap, myCache, next_path, next_path_len);
        int total_steps=steps_current_to_next+steps_next_to_end;
        if (total_steps < lowest_steps)
            lowest_steps = total_steps;
        
        // reset off the next step
        keys_to_get[i]=KEY_NOT_OBTAINED;
    }
    // now we have the lowest steps at this point, so add it to the cache
    insert_cache(myCache, current_position, keys_to_get, lowest_steps);
    return lowest_steps;
}


// current_path is list of keys
int recusrive_work_it(map * map, int * current_path, int current_path_len, int current_path_steps, int best_path_steps)
{
//    printf("best path steps is %d\n", best_path_steps);
    char path[27];
    int avail_steps[MAX_KEYS];
    int next_path[MAX_KEYS];
    int keys_to_get[MAX_KEYS];
    
    for (int i=0; i<MAX_KEYS; i++)
    {
        keys_to_get[i]=map->keys[i]; // mark all of them as they were at the start
        avail_steps[i]=NOT_AVAILABLE;
    }
    
    for (int i=0; i<current_path_len; i++)
    {
        path[i]=current_path[i];
        avail_steps[current_path[i]-MIN_KEY]=VISITED;
        keys_to_get[current_path[i]-MIN_KEY]=KEY_OBTAINED;
        next_path[i]=current_path[i];
    }
    path[current_path_len]='\0';
//    printf("current path length is %d - %s\n", current_path_len, path);
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (avail_steps[i]==VISITED)
        {
//            printf("key %c is visited\n", i+MIN_KEY);
            continue;
        }
        
        if (map->keys[i]==DOES_NOT_EXIST)
            avail_steps[i]=DOES_NOT_EXIST;
        else
        {
            avail_steps[i]=AVAILABLE;
            for (int j=0; j<MAX_KEYS; j++)
            {
                if (map->doors_blocking_keys[i][j]==KEY_REQUIRED && avail_steps[j]!=VISITED)
                {
//                    printf("key %c is not available because key %c has not been visited  door_blocking_keys[i][j]=%d avail-steps[j]=%d\n", i+MIN_KEY, j+MIN_KEY, map->doors_blocking_keys[i][j], avail_steps[j]);
                    avail_steps[i]=NOT_AVAILABLE;
                    break;
                }
            }
//            if (avail_steps[i]==AVAILABLE)
//            {
//                printf("key %c is available\n", i+MIN_KEY);
//            }
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
//            printf("Checking path ");
//            for (int j=0; j<next_path_len; j++)
//            {
//                printf("%c", next_path[j]);
//            }
//            printf("\n");
            int next_path_steps=current_path_steps;
            if (next_path_len==1) // moving from start
            {
                next_path_steps+=map->steps_from_start_to_key[i];
//                printf("Adding %d steps from start to %c to get new total of %d\n", map->steps_from_start_to_key[i], i+MIN_KEY, next_path_steps);
            }
            else
            {
                next_path_steps+=map->steps_from_key_to_key[current_position-MIN_KEY][i];
//                printf("Adding %d steps from %c to %c to get new total of %d\n", map->steps_from_key_to_key[current_position-MIN_KEY][i], current_position, i+MIN_KEY, next_path_steps);
            }
            if (next_path_steps >= best_path_steps)
            {
//                printf("   skipping from here; next path steps %d is greater than best path steps of %d\n", next_path_steps, best_path_steps);
                continue;
            }
            best_path_steps = recusrive_work_it(map, next_path, next_path_len, next_path_steps, best_path_steps);
//            printf("after recursion call, best_path_steps is %d\n", best_path_steps);
        }
    }
    
    if (!has_avail_steps)
    {
        if (best_path_steps > current_path_steps)
            printf("the best path steps under %s is %d\n", path, best_path_steps);
            
        best_path_steps=current_path_steps;
    }
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

void print_multi_map(multi_map * theMap)
{
    printf("Top-Left:\n");
    print_map(&theMap->submaps[0]);
    printf("Top-Right:\n");
    print_map(&theMap->submaps[1]);
    printf("Bottom-Left:\n");
    print_map(&theMap->submaps[2]);
    printf("Bottom-Right:\n");
    print_map(&theMap->submaps[3]);
}

void load_submap(map * parentMap, map * childMap, int startCol, int startRow)
{
    childMap->current_location.col=startCol;
    childMap->current_location.row=startRow;
    
    childMap->max_col=parentMap->max_col;
    childMap->max_row=parentMap->max_row;
    
    for (int col=0; col<parentMap->max_col; col++)
    {
        for (int row=0; row<parentMap->max_row; row++)
        {
            childMap->layout[col][row]=parentMap->layout[col][row];
        }
    }
    
    makeNonReachableAreasWalls(childMap);
    
    for (int col=0; col<parentMap->max_col; col++)
    {
        for (int row=0; row<parentMap->max_row; row++)
        {
            char value=childMap->layout[col][row];
            if (isDoor(value))
            {
                char index=value-MIN_DOOR;
                childMap->doors[index]=DOOR_EXISTS;
                childMap->door_location[index].col=col;
                childMap->door_location[index].row=row;
            }
            else if (isKey(value))
            {
                char index=value-MIN_KEY;
                childMap->keys[index]=KEY_NOT_OBTAINED;
                childMap->key_location[index].col=col;
                childMap->key_location[index].row=row;
            }
        }
    }        
}

void split_map_to_multi_maps(map * parentMap, multi_map * childMaps)
{
    for (int i=0; i<NUM_SUBMAPS; i++)
        initStartMap(&childMaps->submaps[i]);
    // after loading, the parentMap current location will point to the bottom-right @ sign.
    //   need to subtract 1 row and 1 column from that location to get the centerpoint
    int mid_col=parentMap->current_location.col-1;
    int mid_row=parentMap->current_location.row-1;
    printf("mid_col is %d. mid_row is %d\n", mid_col, mid_row);
    
    // load top-left
    load_submap(parentMap, &childMaps->submaps[TOP_LEFT], mid_col-1, mid_row-1);
    
    // load top-right
    load_submap(parentMap, &childMaps->submaps[TOP_RIGHT], mid_col+1, mid_row-1);
    
    // load bottom-left
    load_submap(parentMap, &childMaps->submaps[BOTTOM_LEFT], mid_col-1, mid_row+1);
    
    // load bottom-right. cols from mid_col to parent.max_col, rows from mid_row to parent.max_row
    load_submap(parentMap, &childMaps->submaps[BOTTOM_RIGHT], mid_col+1, mid_row+1);
    
    for (int i=0; i<MAX_KEYS; i++)
    {
        childMaps->door_submaps[i]=NONE;
        childMaps->key_submaps[i]=NONE;
        for (int j=0; j<NUM_SUBMAPS; j++)
        {
            if (childMaps->submaps[j].keys[i]!=DOES_NOT_EXIST)
                childMaps->key_submaps[i]=j;
            if (childMaps->submaps[j].doors[i]!=DOES_NOT_EXIST)
                childMaps->door_submaps[i]=j;
        }
    }
}

void buildAndWorkMultiMaps(multi_map * multiMap, multi_cache * myCache)
{
    for (int i=0; i<NUM_SUBMAPS; i++)
    {
        calculateStartToKeyDistances(&multiMap->submaps[i]);
        calculateKeyToKeyDistances(&multiMap->submaps[i]);
    }
    
    multi_paths myPaths;
    for (int i=0; i<NUM_SUBMAPS; i++)
    {
        myPaths.current_path_lens[i]=0;
    }
    
    int best_steps=recursive_build_multi_cache(multiMap, myCache, &myPaths);
    printf("best_steps after recursion is %d\n", best_steps);
    //makeChildrenMaps(parentMap);
        
    multiMap->best_multi_map_steps=best_steps;
    //deleteChildrenMaps(parentMap);
}

int build_multi_keys_to_get(multi_map * multiMap, multi_paths * paths, int * keys_to_get)
{
    int num_remaining=0;
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (multiMap->key_submaps[i]!=NONE)
        {
            keys_to_get[i]=multiMap->submaps[multiMap->key_submaps[i]].keys[i];
            num_remaining++;
        }
        else
        {
            keys_to_get[i]=DOES_NOT_EXIST;
        }
    }
    for (int j=0; j<NUM_SUBMAPS; j++)
    {
        for (int i=0; i<paths->current_path_lens[j]; i++)
        {
            keys_to_get[paths->current_paths[j][i]-MIN_KEY]=KEY_OBTAINED;
            num_remaining--;
        }
    }
    //printf("returning %d keys to get\n", num_remaining);
    return num_remaining;
}

void dupe_multi_paths(multi_paths * target, multi_paths * source)
{
    for (int i=0; i<NUM_SUBMAPS; i++)
    {
        target->current_path_lens[i]=source->current_path_lens[i];
        for (int j=0; j<target->current_path_lens[i]; j++)
        {
            target->current_paths[i][j]=source->current_paths[i][j];
        }
    }
}

void print_multi_paths(multi_paths * paths)
{
    for (int i=0; i<NUM_SUBMAPS; i++)
    {
        printf("Path %d has %d elements: ", i, paths->current_path_lens[i]);
        for (int j=0; j<paths->current_path_lens[i]; j++)
            printf("%c ", paths->current_paths[i][j]);
        printf("\n");
    }
}

int recursive_build_multi_cache(multi_map * multiMap, multi_cache * myCache, multi_paths * current_multi_paths)
{
    int keys_to_get[26];
    int num_keys_to_get=build_multi_keys_to_get(multiMap, current_multi_paths, keys_to_get);
    int current_position[NUM_SUBMAPS];
    for (int i=0; i<NUM_SUBMAPS; i++)
        current_position[i]=(current_multi_paths->current_path_lens[i]==0?'@':current_multi_paths->current_paths[i][current_multi_paths->current_path_lens[i]-1]);
    //printf("Current path is :");
    //print_multi_paths(current_multi_paths);
    //printf("There are %d keys to get\n", num_keys_to_get);
    
    if (num_keys_to_get==1) // special case - 1 key to get. just insert the distance from the current position to it.
    {
        for (int i=0; i<MAX_KEYS; i++)
        {
            if (keys_to_get[i]==KEY_NOT_OBTAINED)
            {
                int submap=multiMap->key_submaps[i];
                if (current_position[submap]=='@')
                {
                    return multiMap->submaps[submap].steps_from_start_to_key[i];
                }
                else
                {
                    return multiMap->submaps[submap].steps_from_key_to_key[current_position[submap]-MIN_KEY][i];
                }
            }
        }
    }
    // if here, num_keys_to_get is 2 or greater
    //    first check if there is a cache hit
    multi_cache_node * hit = find_multi_cache(myCache, current_position, keys_to_get);
    if (hit != NULL)
        return hit->best_steps;
    // if here, we have a cache miss. Need to recusrively go in, one option at a time.
    multi_paths next_paths;
    int lowest_steps=999999999;
    
    dupe_multi_paths(&next_paths, current_multi_paths);
    
    for (int i=0; i<MAX_KEYS; i++)
    {
        if (keys_to_get[i]!=KEY_NOT_OBTAINED)
            continue;
        //printf("keys_to_get[%c] is KEY_NOT_OBTAINED\n", i+MIN_KEY);

        int not_required_key=0;
        int key_i_submap=multiMap->key_submaps[i];
        
        for (int j=0; j<MAX_KEYS; j++)
        {
            int door_j_submap=multiMap->door_submaps[j];
            int key_j_submap=multiMap->key_submaps[j];
            
            //if (multiMap->submaps[key_i_submap].doors_blocking_keys[i][j]==KEY_REQUIRED)
            //{
            //    printf("key %c is required before key %c\n", j+MIN_KEY, i+MIN_KEY);
            //    printf("keys_to_get[%d] is %d. KEY_OBTAINED is %d\n", j, keys_to_get[j], KEY_OBTAINED);
            ////    //printf(doors_blocking_keys);
            //}
            
            if (key_i_submap == door_j_submap)
            {
                // only care if they interact in the same submap
                if (multiMap->submaps[key_i_submap].doors_blocking_keys[i][j]==KEY_REQUIRED && keys_to_get[j]!=KEY_OBTAINED)
                {
                    //printf("key %c is not available because key %c has not been visited\n", i+MIN_KEY, j+MIN_KEY);
                    not_required_key=1;
                    break;
                }
            }
        }
        
        if (not_required_key==1)
            continue;

        // set up the next step
        int steps_current_to_next=0;
        if (current_position[key_i_submap]=='@')
        {
            steps_current_to_next=multiMap->submaps[key_i_submap].steps_from_start_to_key[i];
        }
        else
        {
            steps_current_to_next=multiMap->submaps[key_i_submap].steps_from_key_to_key[current_position[key_i_submap]-MIN_KEY][i];
        }

        keys_to_get[i]=KEY_OBTAINED;
        
        next_paths.current_paths[key_i_submap][next_paths.current_path_lens[key_i_submap]]=i+MIN_KEY;
        next_paths.current_path_lens[key_i_submap]++;
        
        //printf("Calling recusrively from path: ");
        //for (int i=0; i<current_path_len; i++)
        //    printf("%c", current_path[i]);
        //printf("\n");
        
        int steps_next_to_end=recursive_build_multi_cache(multiMap, myCache, &next_paths);
        int total_steps=steps_current_to_next+steps_next_to_end;
        if (total_steps < lowest_steps)
            lowest_steps = total_steps;
        
        // reset off the next step
        keys_to_get[i]=KEY_NOT_OBTAINED;
        next_paths.current_path_lens[key_i_submap]--;
    }

    // now we have the lowest steps at this point, so add it to the cache
    insert_multi_cache(myCache, current_position, keys_to_get, lowest_steps);
    return lowest_steps;
}

void makeNonReachableAreasWalls(map * map)
{
    int reachable[MAX_MAP_DIMENSION][MAX_MAP_DIMENSION];
    for (int col=0; col<map->max_col; col++)
    {
        for (int row=0; row<map->max_row; row++)
        {
            reachable[col][row]=NOT_REACHABLE;
        }
    }
        
    point location=map->current_location;
    reachable[location.col][location.row]=REACHABLE;
    int workToDo = 1;
    while(workToDo != 0)
    {
        workToDo=0;
        for (int col=0; col<map->max_col; col++)
        {
            for (int row=0; row<map->max_row; row++)
            {
                if (reachable[col][row]==REACHABLE)
                {
                    // up
                    if (row > 0)
                    {
                        if ((!isWall(map, col, row-1, 0)) && (reachable[col][row-1]==NOT_REACHABLE))
                        {
                            workToDo=1;
                            reachable[col][row-1]=REACHABLE;
                        }
                    }
                    // down
                    if (row < map->max_row-1)
                    {
                        if ((!isWall(map, col, row+1, 0)) && (reachable[col][row+1]==NOT_REACHABLE))
                        {
                            workToDo=1;
                            reachable[col][row+1]=REACHABLE;
                        }
                    }
                    // left
                    if (col > 0)
                    {
                        if ((!isWall(map, col-1, row, 0)) && (reachable[col-1][row]==NOT_REACHABLE))
                        {
                            workToDo=1;
                            reachable[col-1][row]=REACHABLE;
                        }
                    }
                    // right
                    if (col < map->max_col-1)
                    {
                        if ((!isWall(map, col+1, row, 0)) && (reachable[col+1][row]==NOT_REACHABLE))
                        {
                            workToDo=1;
                            reachable[col+1][row]=REACHABLE;
                        }
                    }
                }
            }
        }
    }

    for (int col=0; col<map->max_col; col++)
    {
        for (int row=0; row<map->max_row; row++)
        {
            if ((!isWall(map, col, row, 0)) && (reachable[col][row]==NOT_REACHABLE))
            {
                map->layout[col][row]=WALL;
            }
        }
    }
}
