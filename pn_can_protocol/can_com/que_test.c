//
// Created by peter on 12/21/2022.
//
#include "que.h"
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint32_t id;
    uint8_t bytes[8];
}que_test_CanStruct;

void que_test_print_struct(que_test_CanStruct canStruct){
    printf("0x%x: ",canStruct.id);
    for (int i = 0; i < 8; ++i)
        printf("%u ",canStruct.bytes[i]);
    printf("\n");
}

void que_test(){
    que_test_CanStruct st1 = {.id = 0x69,.bytes = {1, 2, 34, 4, 56, 7, 88, 9}};
    que_test_CanStruct st2 = {.id = 0x49,.bytes = {1, 2, 34, 4, 56, 7, 88, 9}};
    que_test_CanStruct st3 = {.id = 0x59,.bytes = {1, 2, 34, 4, 56, 7, 88, 9}};
    que_test_CanStruct st4 = {.id = 0x79,.bytes = {1, 2, 34, 4, 56, 7, 88, 9}};
    que_test_CanStruct st5 = {.id = 0x89,.bytes = {1, 2, 34, 4, 56, 7, 88, 9}};

    Que* que = que_create();
    que_push(que,(void*)&st1);
    que_push(que,(void*)&st2);
    que_push(que,(void*)&st3);
    que_push(que,(void*)&st4);
    que_push(que,(void*)&st5);

    for (int i = 0; i < 5; ++i) {
        que_test_print_struct(*((que_test_CanStruct *) que_get(que)));
        que_pop(que);
    }

    printf("leaked : %d\n", que_getLeakedBytes(que));
    que_delete(que);
    printf("total leaked : %d\n", que_getLeakedBytes(NULL));
}