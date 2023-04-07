/*
 * sync_layer_can.c
 *
 *  Created on: Apr 6, 2023
 *      Author: peter
 */

#include "sync_layer_can.h"
#include "stdarg.h"

#define SYNC_LAYER_CAN_TRANSMIT_TIMEOUT 1000
#define SYNC_LAYER_CAN_RECEIVE_TIMEOUT 1000

#define SYNC_LAYER_CAN_TX_SEND_ACK_RETRY 2
#define SYNC_LAYER_CAN_TX_SEND_RETRY 2

/******************CONSOLE*****************************/
typedef enum {
	CONSOLE_ERROR, CONSOLE_INFO, CONSOLE_WARNING
} ConsoleStatus;
static void console(ConsoleStatus status, const char *func_name,
		const char *msg, ...) {
//	if (status == CONSOLE_INFO)
//		return;
	//TODO make naked and show all registers
	if (status == CONSOLE_ERROR) {
		printf("uart.c|%s> ERROR :", func_name);
	} else if (status == CONSOLE_INFO) {
		printf("uart.c|%s> INFO : ", func_name);
	} else if (status == CONSOLE_WARNING) {
		printf("uart.c|%s> WARNING : ", func_name);
	} else {
		printf("uart.c|%s: ", func_name);
	}
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
}

/*****************PRIVATE COMMON**********************/
static uint32_t timeInMillis() {
	return HAL_GetTick();
}

static uint8_t (*can_send_func)(uint32_t id, uint8_t *bytes, uint8_t len);
static uint8_t canSend(uint32_t id, uint8_t *bytes, uint8_t len) {
	if (can_send_func == NULL) {
		console(CONSOLE_INFO, __func__, "canSendFunc is NULL\n");
		return 0;
	}
	if (can_send_func(id, bytes, len)) {
		console(CONSOLE_INFO, __func__, "CAN send success\n");
		return 1;
	}
	console(CONSOLE_INFO, __func__, "CAN send failed\n");
	return 0;
}

/******************TRANSMIT***************************/
static void (*tx_callback_func)(SyncLayerCanLink *link,
		SyncLayerCanData *data, uint8_t status);
static void txCallback(SyncLayerCanLink *link, SyncLayerCanData *data,
		uint8_t status) {
	if (tx_callback_func == NULL) {
		console(CONSOLE_WARNING, __func__, "txCallBack is NULL\n");
		return;
	}
	tx_callback_func(link, data, status);
}

uint8_t sync_layer_can_txSendThread(SyncLayerCanLink *link,
		SyncLayerCanData *data) {
	uint8_t bytes[8] = { 0 };
	uint8_t is_failed = 0;

	/* Check success */
	if (data->track == SYNC_LAYER_CAN_TRANSMIT_SUCCESS) {
		txCallback(link, data, 1);
		return 1;
	} else if (data->track == SYNC_LAYER_CAN_TRANSMIT_FAILED) {
		if (data->dataRetry > SYNC_LAYER_CAN_TX_SEND_RETRY) {
			console(CONSOLE_ERROR, __func__, "Sending failed exceed limit %d\n",
					data->dataRetry);
			txCallback(link, data, 0);
			return 1;
		} else {
			console(CONSOLE_WARNING, __func__,
					"Send failed and retrying... %d\n", data->dataRetry);
			data->track = SYNC_LAYER_CAN_START_REQUEST;
		}
	}

	/* Check transmit timeout */
	if (data->time_elapse > SYNC_LAYER_CAN_TRANSMIT_TIMEOUT) {
		console(CONSOLE_ERROR, __func__, "Data transmit 0x%0x timeout %d\n",
				data->id, SYNC_LAYER_CAN_TRANSMIT_TIMEOUT);
		data->count = 0;
		data->track = SYNC_LAYER_CAN_START_REQUEST;
	}

	/* Sending retry is less than limit */
	if (data->dataRetry < SYNC_LAYER_CAN_TX_SEND_RETRY) {
		/* Restarting sending process*/
		data->track = SYNC_LAYER_CAN_START_REQUEST;
		console(CONSOLE_WARNING, __func__, "Retrying to send data 0x%0x\n",
				data->id);
	}

	/* Check Track */
	if (data->track == SYNC_LAYER_CAN_START_REQUEST) {
		/* START REQ */

		data->sendAckRetry = 0;
		data->count = 0;
		*(uint32_t*) bytes = data->id;
		*(uint16_t*) ((uint32_t*) bytes + 1) = data->size;
		if (!canSend(link->start_req_ID, bytes, 8)) {
			/* Can sending failed */
			console(CONSOLE_ERROR, __func__,
					"Start request 0x%0x of data 0x%0x send failed\n",
					link->start_req_ID, data->id);
			is_failed = 1;
		} else {
			/* Can sending success */
			data->track = SYNC_LAYER_CAN_START_ACK;
			console(CONSOLE_INFO, __func__,
					"Start request 0x%0x of data 0x%0x send	successful\n",
					link->start_req_ID, data->id);
		}
	} else if (data->track == SYNC_LAYER_CAN_DATA) {
		/* DATA */

		/* Calculate new data size can be transmitted */
		uint16_t new_data_size = data->size - data->count;
		if (new_data_size >= 8)
			new_data_size = 8;

		if (!canSend(data->id, bytes + data->count, new_data_size)) {
			/* Can sending failed */
			console(CONSOLE_ERROR, __func__,
					"Data (%d-%d) of data 0x%0x send failed\n", data->count,
					new_data_size, data->id);
			is_failed = 1;
		} else {
			/* Can sending success */
			data->track = SYNC_LAYER_CAN_DATA_ACK;
			data->count += new_data_size;
			console(CONSOLE_INFO, __func__,
					"Data (%d-%d) of data 0x%0x send success\n", data->count,
					new_data_size, data->id);
		}
	} else if (data->track == SYNC_LAYER_CAN_DATA_COUNT_RESET_REQUEST) {
		/* DATA COUNT RESET REQ */

		*(uint32_t*) bytes = data->id;
		*(uint16_t*) ((uint32_t*) bytes + 1) = data->count;
		if (!canSend(link->data_count_reset_req_ID, bytes, 8)) {
			/* Can sending failed */
			console(CONSOLE_ERROR, __func__,
					"Data count %d reset request 0x%0x of data 0x%0x send failed\n",
					data->count, link->data_count_reset_req_ID, data->id);
			is_failed = 1;
		} else {
			/* Can sending success */
			data->track = SYNC_LAYER_CAN_DATA_COUNT_RESET_ACK;
			console(CONSOLE_INFO, __func__,
					"Data count %d reset request 0x%0x of data 0x%0x send success\n",
					data->count, link->data_count_reset_req_ID, data->id);
		}
	} else if (data->track == SYNC_LAYER_CAN_END_REQUEST) {
		/* END REQ */

		*(uint32_t*) bytes = data->id;
		uint32_t crc = 0x00;	//TODO calculate crc
		*((uint32_t*) bytes + 1) = crc;
		if (!canSend(link->end_req_ID, bytes, 8)) {
			/* Can sending failed */
			console(CONSOLE_ERROR, __func__,
					"Data end request request 0x%0x of data 0x%0x send failed\n",
					link->data_count_reset_req_ID, data->id);
			is_failed = 1;
		} else {
			/* Can sending success */
			data->track = SYNC_LAYER_CAN_END_ACK;
			console(CONSOLE_INFO, __func__,
					"Data end request request 0x%0x of data 0x%0x send success\n",
					link->data_count_reset_req_ID, data->id);
		}
	}

	data->time_elapse = timeInMillis();
	if (data->time_elapse == 0)
		console(CONSOLE_WARNING, __func__, "Time elapse %d\n",
				data->time_elapse);

	if (is_failed) {
		/* Retry sending and ack */
		data->sendAckRetry++;
		console(CONSOLE_WARNING, __func__, "Send Request failed %d\n",
				data->dataRetry);
	}

	if (data->sendAckRetry >= SYNC_LAYER_CAN_TX_SEND_ACK_RETRY) {
		/* Sending and ack retry limit exceeds */
//		console(CONSOLE_WARNING, __func__, "Send failed \n");
		data->dataRetry++;
		data->track = SYNC_LAYER_CAN_TRANSMIT_FAILED;
	}

	return 0;
}

uint8_t sync_layer_can_txReceiveThread(SyncLayerCanLink *link,
		SyncLayerCanData *data, uint32_t can_id, uint8_t *can_bytes,
		uint8_t can_bytes_len) {
	uint32_t data_id;
	uint8_t is_failed = 0;

	if (data->track == SYNC_LAYER_CAN_START_ACK
			&& can_id == link->start_ack_ID) {
		/* START ACK */
		data_id = *(uint32_t*) can_bytes;
		if (data_id != data->id) {
			/* ID doesn't match */
			console(CONSOLE_ERROR, __func__,
					"Start ack 0x%0x of data 0x%0x contains wrong data ID 0x%0x\n",
					link->start_ack_ID, data->id, data_id);
			is_failed = 0;
		} else {
			/* ID matched */
			console(CONSOLE_INFO, __func__,
					"Start ack 0x%0x of data 0x%0x success", link->start_ack_ID,
					data->id);
			data->track = SYNC_LAYER_CAN_DATA;
		}
	} else if (data->track == SYNC_LAYER_CAN_DATA_ACK
			&& can_id == link->data_ack_ID) {
		/* DATA ACK */
		data_id = *(uint32_t*) can_bytes;
		if (data_id != data->id) {
			/* ID doesn't match */
			console(CONSOLE_ERROR, __func__,
					"Data ack 0x%0x of data 0x%0x contains wrong data ID 0x%0x\n",
					link->data_ack_ID, data->id, data_id);
			is_failed = 0;
		} else {
			/* ID matched */
			uint16_t data_count = *(uint16_t*) ((uint32_t*) can_bytes + 1);
			if (data_count > data->count) {
				/* Destination count is greater than actual */
				console(CONSOLE_WARNING, __func__,
						"Data ack 0x%0x of data 0x%0x contains higher count %d then actual %d",
						link->data_ack_ID, data->id, data_count, data->count);
				data->track = SYNC_LAYER_CAN_DATA_COUNT_RESET_REQUEST;
			} else if (data_count < data->count) {
				/* Destination count is smaller than actual */
				console(CONSOLE_WARNING, __func__,
						"Data ack 0x%0x of data 0x%0x contains lower count %d then actual %d",
						link->data_ack_ID, data->id, data_count, data->count);

				console(CONSOLE_WARNING, __func__,
						"Count %d of data 0x%0x is reset to %d", data->count,
						data->id, data_count, data_count);
				data->count = data_count;
				data->track = SYNC_LAYER_CAN_DATA_COUNT_RESET_REQUEST;
			} else {
				/* Destination count and source count is equal */
				console(CONSOLE_INFO, __func__,
						"Start ack 0x%0x of data 0x%0x success",
						link->data_ack_ID, data->id);
				if (data->count < data->size)
					data->track = SYNC_LAYER_CAN_DATA;
				else
					data->track = SYNC_LAYER_CAN_END_REQUEST;
			}
		}
	} else if (data->track == SYNC_LAYER_CAN_DATA_COUNT_RESET_ACK
			&& can_id == link->data_count_reset_ack_ID) {
		/* DATA COUNT RESET ACK */
		data_id = *(uint32_t*) can_bytes;
		if (data_id != data->id) {
			/* ID doesn't match */
			console(CONSOLE_ERROR, __func__,
					"Data count reset ack 0x%0x of data 0x%0x contains wrong data ID 0x%0x\n",
					link->data_count_reset_ack_ID, data->id, data_id);
			is_failed = 0;
		} else {
			/* ID matched */
			console(CONSOLE_INFO, __func__,
					"Data count reset ack 0x%0x of data 0x%0x success",
					link->start_ack_ID, data->id);
			data->track = SYNC_LAYER_CAN_DATA;
		}
	} else if (data->track == SYNC_LAYER_CAN_END_ACK
			&& can_id == link->end_ack_ID) {
		/* END ACK */
		data_id = *(uint32_t*) can_bytes;
		if (data_id != data->id) {
			/* ID doesn't match */
			console(CONSOLE_ERROR, __func__,
					"Data count reset ack 0x%0x of data 0x%0x contains wrong data ID 0x%0x\n",
					link->end_ack_ID, data->id, data_id);
			is_failed = 0;
		} else {
			/* ID matched */
			console(CONSOLE_INFO, __func__,
					"Data count reset ack 0x%0x of data 0x%0x success\n",
					link->end_ack_ID, data->id);
			uint8_t is_transmit_success =
					*(uint8_t*) ((uint32_t*) can_bytes + 1);
			if (is_transmit_success) {
				console(CONSOLE_INFO, __func__, "Data CRC match success\n");
				data->track = SYNC_LAYER_CAN_TRANSMIT_SUCCESS;
			} else {
				console(CONSOLE_WARNING, __func__, "Data CRC match failed\n");
				data->track = SYNC_LAYER_CAN_TRANSMIT_SUCCESS;
			}
		}
	}

	return !is_failed;

}

/******************RECEIVE*********************************/
static void (*rx_callback_func)(SyncLayerCanLink *link,
		SyncLayerCanData *data, uint8_t status);
static void rxCallback(SyncLayerCanLink *link, SyncLayerCanData *data,
		uint8_t status) {
	if (rx_callback_func == NULL) {
		console(CONSOLE_WARNING, __func__, "rxCallBack is NULL\n");
		return;
	}
	rx_callback_func(link, data, status);
}

uint8_t sync_layer_can_rxSendThread(SyncLayerCanLink *link,
		SyncLayerCanData *data) {
	uint8_t bytes[8] = { 0 };
	uint8_t is_failed = 0;

	if (data->track == SYNC_LAYER_CAN_RECEIVE_SUCCESS) {
		rxCallback(link, data, 1);
		return 1;
	} else if (data->track == SYNC_LAYER_CAN_RECEIVE_FAILED) {
		rxCallback(link, data, 0);
	}

	/* Check transmit timeout */
	if (data->time_elapse > SYNC_LAYER_CAN_RECEIVE_TIMEOUT) {
		console(CONSOLE_ERROR, __func__, "Data received 0x%0x timeout %d\n",
				data->id, SYNC_LAYER_CAN_RECEIVE_TIMEOUT);
		data->track = SYNC_LAYER_CAN_RECEIVE_FAILED;
		rxCallback(link, data, 0);	//failed
		return 1;
	}

	/* Check */
	if (data->track == SYNC_LAYER_CAN_START_ACK) {
		/* START ACK */
		*(uint32_t*) bytes = data->id;
		*(uint16_t*) ((uint32_t*) bytes + 1) = data->size;
		if (!canSend(link->start_ack_ID, bytes, 8)) {
			/* Can sending failed */
			console(CONSOLE_ERROR, __func__,
					"Start ack 0x%0x of data 0x%0x send failed\n",
					link->start_ack_ID, data->id);
			is_failed = 1;
		} else {
			/* Can sending success */
			data->track = SYNC_LAYER_CAN_DATA;
			console(CONSOLE_INFO, __func__,
					"Start request ACK 0x%0x of data 0x%0x send successful\n",
					link->start_ack_ID, data->id);
		}
	} else if (data->track == SYNC_LAYER_CAN_DATA_ACK) {
		/* DATA ACK */
		*(uint32_t*) bytes = data->id;
		*(uint16_t*) ((uint32_t*) bytes + 1) = data->count;
		if (!canSend(link->data_ack_ID, bytes, 8)) {
			/* Can sending success */
			console(CONSOLE_ERROR, __func__,
					"Data received ack 0x%0x of data 0x%0x send failed\n",
					link->data_ack_ID, data->id);
			is_failed = 1;
		} else {
			/* Can sending failed */
			console(CONSOLE_INFO, __func__,
					"Data received ack 0x%0x of data 0x%0x send successful\n",
					link->data_ack_ID, data->id);
			if (data->count == data->size) {
				/* Data receive complete */
				data->track = SYNC_LAYER_CAN_END_REQUEST;
			}
		}
	} else if (data->track == SYNC_LAYER_CAN_DATA_COUNT_RESET_ACK) {
		/* DATA COUNT RESET ACK */
		*(uint32_t*) bytes = data->id;
		*(uint16_t*) ((uint32_t*) bytes + 1) = data->count;
		if (!canSend(link->data_count_reset_ack_ID, bytes, 8)) {
			/* Can sending success */
			console(CONSOLE_ERROR, __func__,
					"Data count reset request 0x%0x of data 0x%0x send failed\n",
					link->data_count_reset_ack_ID, data->id);
			is_failed = 1;
		} else {
			/* Can sending failed */
			console(CONSOLE_INFO, __func__,
					"Data count reset request 0x%0x of data 0x%0x send successful\n",
					link->data_count_reset_ack_ID, data->id);
		}
	} else if (data->track == SYNC_LAYER_CAN_END_ACK) {
		*(uint32_t*) bytes = data->id;
		/* END ACK */
		if (!canSend(link->end_ack_ID, bytes, 8)) {
			/* Can sending success */
			console(CONSOLE_ERROR, __func__,
					"Data received ack 0x%0x of data 0x%0x send failed\n",
					link->end_ack_ID, data->id);
			is_failed = 1;
		} else {
			/* Can sending failed */
			data->track = SYNC_LAYER_CAN_RECEIVE_SUCCESS;
			console(CONSOLE_INFO, __func__,
					"Data end ack 0x%0x of data 0x%0x send successful\n",
					link->end_ack_ID, data->id);
		}
	}

	data->time_elapse = timeInMillis();
	if (data->time_elapse == 0)
		console(CONSOLE_WARNING, __func__, "Time elapse %d\n",
				data->time_elapse);

	if (is_failed) {
		console(CONSOLE_ERROR, __func__, "Data 0x%0x received failed\n",
				data->id);
		data->track = SYNC_LAYER_CAN_RECEIVE_FAILED;
	}

	return 0;
}

uint8_t sync_layer_can_rxReceiveThread(SyncLayerCanLink *link,
		SyncLayerCanData *data, uint32_t can_id, uint8_t *can_bytes,
		uint8_t can_bytes_len) {
	uint32_t data_id;
	uint8_t is_failed = 0;
	if (data->track == SYNC_LAYER_CAN_START_REQUEST
			&& can_id == link->start_req_ID) {
		/* START REQ */
		data_id = *(uint32_t*) can_bytes;
		uint16_t size = *(uint16_t*) ((uint32_t*) can_bytes + 1);
		console(CONSOLE_INFO, __func__,
				"Start ack 0x%0x of data 0x%0x success\n", link->start_req_ID,
				data->id);
		data->id = data_id;
		data->count = 0;
		data->size = size;
		data->track = SYNC_LAYER_CAN_START_ACK;
	} else if (data->track == SYNC_LAYER_CAN_DATA && can_id == data->id) {
		/* DATA */
		if (can_bytes_len < 8) {
			for (int i = 0; i < can_bytes_len; i++)
				data->bytes[data->count + i] = can_bytes[i];
		} else {
			*(uint64_t*) (&data->bytes[data->count]) = *(uint64_t*) can_bytes;
		}
		data->count += can_bytes_len;
		data->track = SYNC_LAYER_CAN_DATA_ACK;
		console(CONSOLE_INFO, __func__, "Data 0x%0x receive success\n",
				data->id);
	} else if (data->track == SYNC_LAYER_CAN_DATA
			&& can_id == link->data_count_reset_req_ID) {
		/* DATA COUNT RESET */
		data_id = *(uint32_t*) can_bytes;
		if (can_id != data->id) {
			/* ID doesn't match */
			console(CONSOLE_ERROR, __func__,
					"Data count reset request 0x%0x of data 0x%0x contains wrong ID 0x%0x\n",
					link->data_count_reset_req_ID, data->id, data_id);
			is_failed = 1;
		} else {
			/* ID matched */
			console(CONSOLE_INFO, __func__,
					"Data count reset request 0x%0x of data 0x%0x receive success\n",
					link->data_count_reset_req_ID, data->id);
			uint16_t count = *(uint16_t*) ((uint32_t*) can_bytes + 1);
			data->track = SYNC_LAYER_CAN_DATA_COUNT_RESET_ACK;
			console(CONSOLE_WARNING, __func__,
					"Data 0x%0x count %d reset to %d", data_id, data->count,
					count);
			data->count = count;
		}
	} else if (data->track == SYNC_LAYER_CAN_END_REQUEST
			&& can_id == link->end_ack_ID) {
		/* END */
		data_id = *(uint32_t*) can_bytes;
		if (can_id != data->id) {
			/* ID doesn't match */
			console(CONSOLE_ERROR, __func__,
					"End ack 0x%0x of data 0x%0x contains wrong ID 0x%0x\n",
					link->end_req_ID, data->id, data_id);
			is_failed = 1;
		} else {
			/* ID matched */
			console(CONSOLE_INFO, __func__,
					"End ack 0x%0x of data 0x%0x receive success\n",
					link->end_req_ID, data->id);
			data->track = SYNC_LAYER_CAN_END_ACK;
		}
	}

	if (is_failed) {
		data->track = SYNC_LAYER_CAN_RECEIVE_FAILED;
		return 0;
	}
	return 1;
}

/*******************COMMON****************************/

void sync_layer_can_init(uint8_t (*canSendFunc)(uint32_t id, uint8_t *bytes, uint8_t len),
		void (*txCallbackFunc)(SyncLayerCanLink *link,
				SyncLayerCanData *data, uint8_t status),
		void (*rxCallbackFunc)(SyncLayerCanLink *link,
				SyncLayerCanData *data, uint8_t status)) {
	can_send_func = canSendFunc;
	tx_callback_func = txCallbackFunc;
	rx_callback_func = rxCallbackFunc;
}

