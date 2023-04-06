//
// Created by niruja on 1/31/2023.
//

#ifndef COMMUNICATION_SYNC_LAYER_CAN_H
#define COMMUNICATION_SYNC_LAYER_CAN_H

#include "stdint.h"
#include "stdio.h"

typedef enum{
    SYNC_LAYER_CAN_TRANSMIT_OK,
    SYNC_LAYER_CAN_TRANSMIT_START_REQ_FAILED,
    SYNC_LAYER_CAN_TRANSMIT_START_ACK_FAILED,
    SYNC_LAYER_CAN_TRANSMIT_DATA_TRANSMIT_FAILED,
    SYNC_LAYER_CAN_TRANSMIT_END_REQ_FAILED,
}SyncLayerCanTransmitStatus;

typedef enum{
    SYNC_LAYER_CAN_RECEIVE_OK,
    SYNC_LAYER_CAN_RECEIVE_START_ACK_FAILED,
    SYNC_LAYER_CAN_RECEIVE_DATA_ACK_FAILED,
    SYNC_LAYER_CAN_RECEIVE_END_ACK_FAILED,
}SyncLayerCanReceiveStatus;

typedef struct{
    uint32_t start_req_ID;
    uint32_t start_ack_ID;
    uint32_t data_ack_ID;
    uint32_t end_req_ID;
    uint32_t end_ack_ID;
}SyncLayerCanLink;

typedef struct{
    uint32_t id;
    uint8_t* bytes;
    uint16_t size;
    uint8_t oneShot;        // 1 for one data ack, 0 for each time data ack
    uint16_t count;         //initialization not required
    uint32_t time_elapse;   //initialization not required
}SyncLayerCanData;

//////////////////////////////////TRANSMITTING////////////////////////////
void sync_layer_can_tx_init(uint32_t (*)(),uint8_t (*)(uint32_t, uint8_t*, uint8_t),void (*)(SyncLayerCanLink*, SyncLayerCanData*, SyncLayerCanTransmitStatus));
void sync_layer_can_tx_send(SyncLayerCanLink *, SyncLayerCanData *);
uint32_t sync_layer_can_tx_recDataId(SyncLayerCanLink*, uint32_t, uint8_t*);
void sync_layer_can_tx_rec(SyncLayerCanLink* , SyncLayerCanData*, uint32_t, uint8_t*, uint8_t);

/////////////////////////////////RECEIVING////////////////////////////////
void sync_layer_can_rx_init(uint32_t (*)(),uint8_t (*)(uint32_t , uint8_t* , uint8_t ),void (*)(SyncLayerCanLink*, SyncLayerCanData*, SyncLayerCanReceiveStatus));
uint32_t sync_layer_can_rx_recDataId(SyncLayerCanLink* , uint32_t, uint8_t*);
void sync_layer_can_rx_rec(SyncLayerCanLink* , SyncLayerCanData *, uint32_t , uint8_t* , uint8_t);

/////////////////////////////////TEST////////////////////////////////////
void sync_layer_can_test();

#endif //COMMUNICATION_SYNC_LAYER_CAN_H
