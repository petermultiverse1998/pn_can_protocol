//
// Created by peter on 2/5/2023.
//

#ifndef COMMUNICATION_MAP_HANDLING_LAYER_H
#define COMMUNICATION_MAP_HANDLING_LAYER_H
#include "stdint.h"
#include "sync_layer_can.h"


//TRANSMITTING
void can_com_init_tx(uint32_t (*)(), uint8_t (*)(uint32_t, uint8_t *, uint8_t),int(*)(const char *__restrict, ...),uint8_t);
void can_com_addLink_tx(uint32_t ,SyncLayerCanLink *);
void can_com_addMessage_tx(uint32_t,uint32_t, uint8_t *, uint16_t, uint8_t);
void can_com_receive_tx(uint32_t, uint32_t, uint8_t *, uint8_t);
void can_com_timeoutMonitor_tx(uint32_t);
void can_com_errorHandler_tx(uint32_t);

//RECEIVING
void can_com_init_rx(uint32_t (*)(), uint8_t (*)(uint32_t, uint8_t *, uint8_t),void (*)(uint32_t,uint32_t,uint16_t),int(*)(const char *__restrict, ...));
void can_com_addLink_rx(uint32_t,SyncLayerCanLink *);
void can_com_receive_rx(uint32_t , uint32_t, uint8_t *, uint8_t);
void can_com_timeoutMonitor_rx(uint32_t);
void can_com_getBytes_rx(uint32_t ,uint32_t,uint8_t*);

//TESTING
void map_handling_layer_test();

#endif //COMMUNICATION_MAP_HANDLING_LAYER_H
