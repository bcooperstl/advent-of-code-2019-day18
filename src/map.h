#ifndef __MAP_H__
#define __MAP_H__

#define MAX_MAP_DIMENSION 96
#define MAX_KEYS 26

#define DOES_NOT_EXIST 0
#define KEY_OBTAINED 1
#define KEY_NOT_OBTAINED 2
#define DOOR_EXISTS 3

#define KEY_NOT_REQUIRED 0
#define KEY_REQUIRED 1

#define VISITED 4
#define AVAILABLE 5
#define NOT_AVAILABLE 6

#define NOT_WORKED -1

#define REACHABLE 1
#define NOT_REACHABLE 2

#define WALL '#'
#define SPACE '.'
#define ME '@'
#define MIN_KEY 'a'
#define MAX_KEY 'z'
#define MIN_DOOR 'A'
#define MAX_DOOR 'Z'

#define NUM_SUBMAPS 4
#define TOP_LEFT 0
#define TOP_RIGHT 1
#define BOTTOM_LEFT 2
#define BOTTOM_RIGHT 3
#define NONE -1

typedef struct cache cache;
typedef struct multi_cache multi_cache;

struct point {
    int col;
    int row;
};

typedef struct point point;


struct map
{
    char layout[MAX_MAP_DIMENSION][MAX_MAP_DIMENSION]; // layout is column, row
    int max_row;
    int max_col;
    point current_location;
    int doors[MAX_KEYS]; // 26 character array for A-Z
    point door_location[MAX_KEYS];
    int keys[MAX_KEYS]; // 26 character array for a-z
    int doors_blocking_keys[MAX_KEYS][MAX_KEYS]; // if doors_blocking_keys[a][b] is 1, that means door B blocks key a, so key b must be picked up before key a
    point key_location[MAX_KEYS];
    int steps_to_key[MAX_KEYS];
    struct map * parent;
    int steps_from_parent;
    int steps_from_start;
    int best_child_map_steps;
    struct map * child_by_key[MAX_KEYS];
    int steps_from_start_to_key[MAX_KEYS];
    int steps_from_key_to_key[MAX_KEYS][MAX_KEYS]; // order is from, to
};

typedef struct map map;

struct multi_map
{
    struct map submaps[NUM_SUBMAPS];
    int key_submaps[MAX_KEYS];
    int door_submaps[MAX_KEYS];
    int best_multi_map_steps;
};

typedef struct multi_map multi_map;

struct multi_paths
{
    int current_paths[NUM_SUBMAPS][MAX_KEYS];
    int current_path_lens[NUM_SUBMAPS];
};

typedef struct multi_paths multi_paths;

int isKey(char ch);
int isDoor(char ch);
int hasKey(map * map, char door);
int isWall(map * map, int col, int row, int doors_as_walls);
int allKeysObtained(map * map);
void calculateStartToKeyDistances(map * map);
void calculateKeyToKeyDistances(map * map);
map * dupeForChildMap(map * map);
void makeChildrenMaps(map * map);
void buildAndWorkChildrenMaps(map * parentMap, int level, cache * myCache);
void deleteChildrenMaps(map * parentMap);
int findBestMapSteps(map * parentMap);
void initStartMap(map * map);
void print_map(map * map);
void print_multi_map(multi_map * map);
int build_keys_to_get(map * parentMap, int * current_path, int current_path_len, int * keys_to_get);
int recusrive_work_it(map * map, int * current_path, int current_path_len, int current_path_steps, int best_path_steps);
int recursive_build_cache(map * parentMap, cache * myCache, int * current_path, int current_path_len);

void split_map_to_multi_maps(map * parentMap, multi_map * childMaps);
int build_multi_keys_to_get(multi_map * multiMap, multi_paths * paths, int * keys_to_get);
void buildAndWorkMultiMaps(multi_map * multiMap, multi_cache * myCache);
void dupe_multi_paths(multi_paths * target, multi_paths * source);
int recursive_build_multi_cache(multi_map * multiMap, multi_cache * myCache, multi_paths * current_paths);
void makeNonReachableAreasWalls(map * map);

#endif
