#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"

int main (int argc, char * argv[])
{
    FILE * infile=NULL;
    char row_buffer[MAX_MAP_DIMENSION+1];
    
    memset(&row_buffer, '\0', sizeof(row_buffer));
    
    map startMap;
    initStartMap(&startMap);
    
    map * bestMap = NULL;
    
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        exit(1);
    }
    
    fprintf(stderr, "Opening file %s\n", argv[1]);
    
    infile=fopen(argv[1], "r");
    if (infile == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", argv[1]);
        exit(1);
    }
    
    while (fgets(row_buffer, sizeof(row_buffer), infile))
    {
        char * eol = strchr(row_buffer, '\r');
        if (eol)
        {
            *eol='\0';
        }
        eol = strchr(row_buffer, '\n');
        if (eol)
        {
            *eol='\0';
        }
        
        int len = strlen(row_buffer);
        startMap.max_col=len;
        for (int i=0; i<len; i++)
        {
            startMap.layout[i][startMap.max_row]=row_buffer[i];
            if (row_buffer[i]=='@')
            {
                startMap.current_location.col=i;
                startMap.current_location.row=startMap.max_row;
            }
            else if (isDoor(row_buffer[i]))
            {
                char index=row_buffer[i]-MIN_DOOR;
                startMap.doors[index]=DOOR_EXISTS;
                startMap.door_location[index].col=i;
                startMap.door_location[index].row=startMap.max_row;
            }
            else if (isKey(row_buffer[i]))
            {
                char index=row_buffer[i]-MIN_KEY;
                startMap.keys[index]=KEY_NOT_OBTAINED;
                startMap.key_location[index].col=i;
                startMap.key_location[index].row=startMap.max_row;
            }
        }   
        startMap.max_row++;
    }
    
    fclose(infile);
    
    print_map(&startMap);
    
    buildAndWorkChildrenMaps(&startMap,0);
    //bestMap=findBestMap(&startMap);
    
    printf("The best distance is %d\n", startMap.best_child_map_steps);
    
    deleteChildrenMaps(&startMap);
    
    return 0;
}
