#ifndef __MAP_H__
#define __MAP_H__

#define MAX_MAP_DIMENSION 96
#define MAX_KEYS 26

#define DOES_NOT_EXIST 0
#define KEY_OBTAINED 1
#define KEY_NOT_OBTAINED 2
#define DOOR_EXISTS 3

#define NOT_WORKED -1

#define WALL '#'
#define SPACE '.'
#define ME '@'
#define MIN_KEY 'a'
#define MAX_KEY 'z'
#define MIN_DOOR 'A'
#define MAX_DOOR 'Z'

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
    point key_location[MAX_KEYS];
    int steps_to_key[MAX_KEYS];
    struct map * parent;
    int steps_from_parent;
    int steps_from_start;
    struct map * child_by_key[MAX_KEYS];
};

typedef struct map map;

int isKey(char ch);
int isDoor(char ch);
int hasKey(map * map, char door);
int isWall(map * map, int col, int row);
int allKeysObtained(map * map);
void calculateKeyDistances(map * map);
map * dupeForChildMap(map * map);
void makeChildrenMaps(map * map);
void buildAndWorkChildrenMaps(map * parentMap, int level);
void deleteChildrenMaps(map * parentMap);
map * findBestMap(map * parentMap);
void initStartMap(map * map);
void print_map(map * map);



#endif
