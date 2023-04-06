//
// Created by peter on 12/24/2022.
//
#include "map.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

typedef struct{
    uint8_t len;
    uint32_t can_id;
    uint8_t *data;
}map_test_Data;

void map_test_print(Map *map){
    for(int id=1;id<=3;id++){
        map_test_Data* data = map_get(map,id);
        printf("%x [",data->can_id);
        for(int i=0;i<data->len;i++)
            printf(" %x",data->data[i]);
        printf(" ]\n");
    }
}

void map_test() {
    Map *map = map_create();

    map_test_Data data1= {.can_id = 1,.len=4};
    data1.data = malloc(4);
    map_put(map,1,&data1);

    map_test_Data data2= {.can_id = 2,.len=6};
    data2.data = malloc(6);
    map_put(map,2,&data2);

    map_test_Data data3= {.can_id = 3,.len=8};
    data3.data = malloc(8);
    map_put(map,3,&data3);

    printf("Before Editing:\n");
    map_test_print(map);

    memset(((map_test_Data*)map_get(map,1))->data,1,4);
    memset(((map_test_Data*)map_get(map,2))->data,2,6);
    memset(((map_test_Data*)map_get(map,3))->data,3,8);

    free(data1.data);
    free(data2.data);
    free(data3.data);

    uint32_t keys[map->size];
    map_getKeys(map, (MapKey *) keys);
    printf("Keys :[ ");
    for(int i=0;i<map->size;i++)
        printf("%d ",keys[i]);
    printf("]\n");

    printf("After Editing:\n");
    map_test_print(map);

    map_remove(map,2);
    map_remove(map,1);
    map_remove(map,3);

    printf("leaked : %d\n", map_getLeakedBytes(map));
    map_delete(map);
    printf("total leaked : %d\n", map_getLeakedBytes(NULL));
}