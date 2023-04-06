//
// Created by peter on 2/14/2023.
//
#include <stdint.h>
#include <malloc.h>
#include "base_layer_can.h"
#include "que.h"

static Que *transmit_que = NULL;
static Que *receive_que = NULL;
static uint32_t transmit_memory_leak_track;
static uint32_t receive_memory_leak_track;

static uint8_t (*can_send)(uint32_t, uint8_t*, uint8_t) = NULL;
static void (*can_receive)(uint32_t, uint8_t*, uint8_t) = NULL;
static int(*debugFunc)(const char *__restrict, ...) = NULL;

static void debug(const char func_name[], const char title[], const char msg[]) {
	if(debugFunc!=NULL)
		debugFunc("BASE LAYER|%s->%s: %s\n", func_name, title, msg);
}

/**
 * Initiation
 * @param canSendFunc       : This function send can message
 * @param canReceiveFunc    : This function is called when data received available
 * @param printf			: Debug function (can be null)
 */
void base_layer_can_init(uint8_t (*canSendFunc)(uint32_t, uint8_t*, uint8_t),
		void (*canReceiveFunc)(uint32_t, uint8_t*, uint8_t),int(*printf)(const char *__restrict, ...)) {
    transmit_memory_leak_track = 0;
    receive_memory_leak_track = 0;
    if (canSendFunc != NULL)
		can_send = canSendFunc;
	if (canReceiveFunc != NULL)
		can_receive = canReceiveFunc;
	debugFunc = printf;

	if (transmit_que == NULL)
		transmit_que = que_create();
	if (receive_que == NULL)
		receive_que = que_create();
	if (transmit_que == NULL)
		debug(__FUNCTION__, "TRANSMIT QUE", "NULL(Heap is Full)");
	if (receive_que == NULL)
		debug(__FUNCTION__, "RECEIVE QUE", "NULL(Heap is Full)");
}

/**
 * This will add can message in transmit que
 * @param id        : can ID
 * @param bytes     : bytes
 * @param len       : length of data
 * @return          : 1 if successfully added in que
 *                  : 0 if failed to add in que
 */
uint8_t base_layer_can_send(uint32_t id, uint8_t *bytes, uint8_t len) {
	if (transmit_que == NULL) { //if transmit que doesn't exits
		debug(__FUNCTION__, "TRANSMIT QUE", "NULL(Heap is Full)");
		return 0;
	}
	if (transmit_que->size >= BASE_LAYER_CAN_TRANSMIT_QUE_SIZE) { //MAX SIZE
		debug(__FUNCTION__, "TRANSMIT QUE", "Full");
		return 0;
	}
	if (id == 0) { //ID can't be zero
		debug(__FUNCTION__, "CAN ID", "Zero(Can't be zero)");
		return 0;
	}
	if (bytes == NULL) { //bytes can't be null
		debug(__FUNCTION__, "BYTES", "NULL(Can't be null)");
		return 0;
	}
	if (len > 8) { //length can't be greater than 8
		debug(__FUNCTION__, "LENTGH", "Greater than 8 (Max value is 8)");
		return 0;
	}
	if (len == 0) //if length is 0 no need to send anything
		return 1;
	int size = sizeof(BaseLayerCanData);
	BaseLayerCanData *data = (BaseLayerCanData*) malloc(size);
    transmit_memory_leak_track+=size;
	if (data == NULL) { //Memory is full
		debug(__FUNCTION__, "DATA", "NULL(Heap is full)");
		return 0;
	}
	data->id = id;
	data->bytes = malloc(8);
    if(data->bytes==NULL) {
        debug(__FUNCTION__, "BYTES MALLOC", "NULL(Heap is full)");
        return 0;
    }
    transmit_memory_leak_track+=8;
	data->len = len;
	*(uint64_t*) data->bytes = *(uint64_t*) bytes;
	if (que_push(transmit_que, data) == NULL) { //memory is full
		debug(__FUNCTION__, "TRANSMIT QUE PUSH", "FAILED(Heap is full)");
		return 0;
	}
	return 1;
}

/**
 * This will add can message in receive que
 * @param id        : can ID
 * @param bytes     : bytes
 * @param len       : length of data
 * @return          : 1 if successfully added in que
 *                  : 0 if failed to add in que
 */
uint8_t base_layer_can_receive(uint32_t id, uint8_t *bytes, uint8_t len) {
	if (receive_que == NULL) { //if receive que doesn't exits
		debug(__FUNCTION__, "RECEIVE QUE", "NULL(Heap is Full)");
		return 0;
	}
	if (receive_que->size >= BASE_LAYER_CAN_RECEIVE_QUE_SIZE) { //MAX SIZE
		debug(__FUNCTION__, "RECEIVE QUE", "Full");
		return 0;
	}
	if (id == 0) { //ID can't be zero
		debug(__FUNCTION__, "CAN ID", "Zero(Can't be zero)");
		return 0;
	}
	if (bytes == NULL) { //bytes can't be null
		debug(__FUNCTION__, "BYTES", "NULL(Can't be null)");
		return 0;
	}
	if (len > 8) { //length can't be greater than 8
		debug(__FUNCTION__, "LENTGH", "Greater than 8 (Max value is 8)");
		return 0;
	}
	if (len == 0) //if length is 0 no need to send anything
		return 1;
	int size = sizeof(BaseLayerCanData);
	BaseLayerCanData *data = (BaseLayerCanData*) malloc(size);
    receive_memory_leak_track+=size;
	if (data == NULL) { //Memory is full
		debug(__FUNCTION__, "DATA", "NULL(Heap is full)");
		return 0;
	}
	data->id = id;
	data->bytes = malloc(8);
    if(data->bytes==NULL) {
        debug(__FUNCTION__, "BYTES MALLOC", "NULL(Heap is full)");
        return 0;
    }
    receive_memory_leak_track+=8;
	data->len = len;
	*(uint64_t*) data->bytes = *(uint64_t*) bytes;
	if (que_push(receive_que, data) == NULL) { //memory is full
		debug(__FUNCTION__, "RECEIVE QUE PUSH", "FAILED(Heap is full)");
		return 0;
	}
	return 1;
}

/**
 * This should be called in timer continuously to monitor
 */
void base_layer_can_send_monitor() {
	if (transmit_que->size <= 0)
		return;
	if (can_send == NULL) {
		debug(__FUNCTION__, "CAN SEND FUNCTION", "NULL");
		return;
	}
	BaseLayerCanData *data = que_get(transmit_que);
	if (data == NULL) {
		debug(__FUNCTION__, "DATA", "NULL");
		return;
	}
    if (!can_send(data->id, data->bytes, data->len))
        return;
	que_pop(transmit_que);
	free(data->bytes);
    transmit_memory_leak_track-=8;
	free(data);
    transmit_memory_leak_track-= sizeof(BaseLayerCanData);
}

/**
 * This should be called in timer continuously to monitor
 */
void base_layer_can_receive_monitor() {
	if (receive_que->size <= 0)
		return;
	if (can_receive == NULL) {
		debug(__FUNCTION__, "CAN SEND FUNCTION", "NULL");
		return;
	}
	BaseLayerCanData *data = que_get(receive_que);
	if (data == NULL) {
		debug(__FUNCTION__, "DATA", "NULL");
		return;
	}
	can_receive(data->id, data->bytes, data->len);
	que_pop(receive_que);
	free(data->bytes);
    receive_memory_leak_track-=8;
	free(data);
    receive_memory_leak_track-=sizeof(BaseLayerCanData);
}

