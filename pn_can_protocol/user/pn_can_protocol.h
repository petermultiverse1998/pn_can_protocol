/*
 * pn_can_protocol.h
 *
 *  Created on: Apr 6, 2023
 *      Author: peter
 */

#ifndef PN_CAN_PROTOCOL_H_
#define PN_CAN_PROTOCOL_H_

#include "main.h"


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

void pn_can_protocol_init(uint8_t (*can_send_func)(uint32_t id, uint8_t *bytes, uint8_t len));
void pn_can_protocol_loop();

#endif /* PN_CAN_PROTOCOL_H_ */
