//
// Created by peter on 2/14/2023.
//

#ifndef COMMUNICATION_BASE_LAYER_CAN_H
#define COMMUNICATION_BASE_LAYER_CAN_H

#include "stdint.h"

#define BASE_LAYER_CAN_TRANSMIT_QUE_SIZE 10000
#define BASE_LAYER_CAN_RECEIVE_QUE_SIZE 10000


typedef struct{
    uint32_t id;
    uint8_t *bytes;
    uint8_t len;
}BaseLayerCanData;

void base_layer_can_init(uint8_t (*)(uint32_t,uint8_t*,uint8_t),void (*)(uint32_t,uint8_t*,uint8_t),int(*)(const char *__restrict, ...));
uint8_t base_layer_can_send(uint32_t ,uint8_t *,uint8_t );
uint8_t base_layer_can_receive(uint32_t ,uint8_t *,uint8_t );
void base_layer_can_send_monitor();
void base_layer_can_receive_monitor();

void base_layer_can_test();

#endif //COMMUNICATION_BASE_LAYER_CAN_H
