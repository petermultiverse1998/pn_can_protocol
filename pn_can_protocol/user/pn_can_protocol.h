/*
 * pn_can_protocol.h
 *
 *  Created on: Apr 6, 2023
 *      Author: peter
 */

#ifndef PN_CAN_PROTOCOL_H_
#define PN_CAN_PROTOCOL_H_

#include "stdint.h"
#include "sync_layer_can.h"

uint8_t pn_can_protocol_addLink(SyncLayerCanLink *link,
		uint8_t (*canSendFunc)(uint32_t id, uint8_t *bytes, uint8_t len),
		uint8_t (*txCallbackFunc)(uint32_t id,uint8_t*bytes,uint16_t size,uint8_t status),
		uint8_t (*rxCallbackFunc)(uint32_t id,uint8_t*bytes,uint16_t size,uint8_t status),uint8_t is_que);
uint8_t pn_can_protocol_addTxMessage(SyncLayerCanLink *link, uint32_t id, uint8_t *data, uint16_t size);
uint8_t pn_can_protocol_pop(SyncLayerCanLink *link);
uint8_t pn_can_protocol_addTxMessagePtr(SyncLayerCanLink *link, uint32_t id, uint8_t *data, uint16_t size);
uint8_t pn_can_protocol_addRxMessagePtr(SyncLayerCanLink *link, uint32_t id, uint8_t *data, uint16_t size);
void pn_can_protocol_sendThread(SyncLayerCanLink* link);
void pn_can_protocol_recThread(SyncLayerCanLink* link,uint32_t id,uint8_t* bytes,uint16_t len);


#endif /* PN_CAN_PROTOCOL_H_ */
