//
// Created by niruja on 1/31/2023.
//

#ifndef COMMUNICATION_SYNC_LAYER_CAN_H
#define COMMUNICATION_SYNC_LAYER_CAN_H

#include "stdint.h"
#include "main.h"

typedef enum {
	SYNC_LAYER_CAN_START_REQUEST,
	SYNC_LAYER_CAN_START_ACK,
	SYNC_LAYER_CAN_DATA,
	SYNC_LAYER_CAN_DATA_COUNT_RESET_REQUEST,
	SYNC_LAYER_CAN_DATA_COUNT_RESET_ACK,
	SYNC_LAYER_CAN_DATA_ACK,
	SYNC_LAYER_CAN_END_REQUEST,
	SYNC_LAYER_CAN_END_ACK,
	SYNC_LAYER_CAN_TRANSMIT_SUCCESS,
	SYNC_LAYER_CAN_RECEIVE_SUCCESS,
	SYNC_LAYER_CAN_TRANSMIT_FAILED,
	SYNC_LAYER_CAN_RECEIVE_FAILED
} SyncLayerCanTrack;

typedef struct {
	uint32_t start_req_ID;
	uint32_t start_ack_ID;
	uint32_t data_count_reset_req_ID;
	uint32_t data_count_reset_ack_ID;
	uint32_t data_ack_ID;
	uint32_t end_req_ID;
	uint32_t end_ack_ID;
} SyncLayerCanLink;

typedef struct {
	uint32_t id;
	uint8_t *bytes;
	uint16_t size;
	SyncLayerCanTrack track; // Initially should SYNC_LAYER_CAN_START_REQUEST
	uint8_t oneShot;        // 1 for one data ack, 0 for each time data ack
	uint16_t count;         // Initialization not required
	uint32_t time_elapse;   // Initialization not required
	uint8_t sendAckRetry;
	uint8_t dataRetry;
} SyncLayerCanData;

//////////////////////////////////TRANSMITTING////////////////////////////
uint8_t sync_layer_can_txSendThread(SyncLayerCanLink *link,
		SyncLayerCanData *data);
uint8_t sync_layer_can_txReceiveThread(SyncLayerCanLink *link,
		SyncLayerCanData *data, uint32_t can_id, uint8_t *can_bytes,
		uint8_t can_bytes_len);

/////////////////////////////////RECEIVING////////////////////////////////
uint8_t sync_layer_can_rxSendThread(SyncLayerCanLink *link,
		SyncLayerCanData *data);
uint8_t sync_layer_can_rxReceiveThread(SyncLayerCanLink *link,
		SyncLayerCanData *data, uint32_t can_id, uint8_t *can_bytes,
		uint8_t can_bytes_len);

////////////////////////////////COMMON////////////////////////////////////
void sync_layer_can_init(
		uint8_t (*canSendFunc)(uint32_t id, uint8_t *bytes, uint8_t len),
		void (*txCallbackFunc)(SyncLayerCanLink *link, SyncLayerCanData *data,
				uint8_t status),
		void (*rxCallbackFunc)(SyncLayerCanLink *link, SyncLayerCanData *data,
				uint8_t status));

/////////////////////////////////TEST////////////////////////////////////
void sync_layer_can_test();

#endif //COMMUNICATION_SYNC_LAYER_CAN_H
