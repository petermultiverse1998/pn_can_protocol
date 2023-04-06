//
// Created by peter on 2/14/2023.
//
#include <stdio.h>
#include "base_layer_can.h"
#include "stdint.h"

uint8_t canSend(uint32_t id,uint8_t*bytes,uint8_t len){
    printf("Transmitted-> 0x%02x : ", id);
    for (int i = 0; i < len; ++i)
        printf("%d ", bytes[i]);
    printf("\n");

    return base_layer_can_receive(id,bytes,len);
//    return 1;
}
void canReceive(uint32_t id,uint8_t*bytes,uint8_t len){
    printf("Received-> 0x%02x : ", id);
    for (int i = 0; i < len; ++i)
        printf("%d ", bytes[i]);
    printf("\n");
}

void base_layer_can_test(){
    base_layer_can_init(canSend, canReceive,NULL);

    //Transmit data
    uint8_t data1[] = {1,2,3,4,5,6,7,8};
    uint8_t data2[] = {1,2,3,4,5};

    //Add data in que
    base_layer_can_send(0xA,data1,8);
    base_layer_can_send(0xB,data2,5);

    //transmit monitor
    for(int i=0;i<10;i++)
        base_layer_can_send_monitor();

    //receive monitor
    for(int i=0;i<10;i++)
        base_layer_can_receive_monitor();
}
