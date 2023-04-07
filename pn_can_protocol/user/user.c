/*
 * user.c
 *
 *  Created on: Apr 6, 2023
 *      Author: peter
 */

#include "user.h"
#include "sync_layer_can.h"
#include "stdlib.h"

/********************CONSOLE***************************/
static void console(const char *title, const char *msg) {
	printf("%s:: %s\n", title, msg);
}

static SyncLayerCanLink links = { 1, 2, 3, 4, 5, 6, 7 };
static SyncLayerCanData sync_data;

/************************CAN****************************/
extern CAN_HandleTypeDef hcan;
static void canInit() {
	CAN_FilterTypeDef can_filter;

	can_filter.FilterActivation = CAN_FILTER_ENABLE;
	can_filter.FilterBank = 0;
	can_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	can_filter.FilterIdHigh = 0x0;
	can_filter.FilterIdLow = 0x0;
	can_filter.FilterMaskIdHigh = 0x0;
	can_filter.FilterMaskIdLow = 0x0;
	can_filter.FilterMode = CAN_FILTERMODE_IDMASK;
	can_filter.FilterScale = CAN_FILTERSCALE_32BIT;
	can_filter.SlaveStartFilterBank = 0;

	HAL_CAN_ConfigFilter(&hcan, &can_filter);
	if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING)
			!= HAL_OK)
		console("CAN RX Interrupt Activate", "Failed");
	if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
		console("CAN TX Interrupt Activate", "Failed");
	if (HAL_CAN_Start(&hcan) != HAL_OK)
		console("CAN Start", "Failed");
}

static CAN_RxHeaderTypeDef rx_header;
static uint8_t data[8];
void canRxInterrupt() {
	HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &rx_header, data);
	sync_layer_can_txReceiveThread(&links, &sync_data, rx_header.ExtId, data,
			rx_header.DLC);
//	sync_layer_can_rxReceiveThread(&links, &sync_data, rx_header.ExtId, data,
//			rx_header.DLC);

//	printf("Interrupt-> 0x%02x : ", (unsigned int) rx_header.ExtId);
//	for (int i = 0; i < rx_header.DLC; ++i)
//		printf("%d ", data[i]);
//	printf("\n");
}

static CAN_TxHeaderTypeDef tx_header;
static uint32_t tx_mailbox;
static uint8_t canSend(uint32_t id, uint8_t *bytes, uint8_t len) {
	tx_header.DLC = len;
	tx_header.ExtId = id;
	tx_header.IDE = CAN_ID_EXT;
	tx_header.RTR = CAN_RTR_DATA;
	tx_header.TransmitGlobalTime = DISABLE;

	printf("canSend-> 0x%02x : ", (unsigned int) id);
	for (int i = 0; i < len; ++i)
		printf("%d ", bytes[i]);
	printf("\n");

	return HAL_CAN_AddTxMessage(&hcan, &tx_header, bytes, &tx_mailbox) == HAL_OK;
}

/**********************MAIN THREAD****************************/
static void txCallback(SyncLayerCanLink *link, SyncLayerCanData *data,
		uint8_t status) {

	if (status)
		console("Data transmit", "Success");
	else
		console("Data transmit", "Failed");
}

static void rxCallback(SyncLayerCanLink *link, SyncLayerCanData *data,
		uint8_t status) {
	if (status)
		console("Data receive", "Success");
	else
		console("Data receive", "Failed");
}

void init() {
	canInit();
	console("INIT", "SUCCESS");
	sync_layer_can_init(canSend, txCallback, rxCallback);

	sync_data.id = 0xA;
	sync_data.count = 0;
	sync_data.sendAckRetry = 2;
	sync_data.dataRetry = 2;
	sync_data.track = SYNC_LAYER_CAN_START_REQUEST;
	sync_data.size = 8;
	sync_data.bytes = (uint8_t*) malloc(8);

	sync_data.bytes[0] = 'H';
	sync_data.bytes[1] = 'e';
	sync_data.bytes[2] = 'l';
	sync_data.bytes[3] = 'l';
	sync_data.bytes[4] = 'o';
	sync_data.bytes[5] = '!';
	sync_data.bytes[6] = '!';
	sync_data.bytes[7] = '!';
}

void loop() {
	sync_layer_can_txSendThread(&links, &sync_data);
//	sync_layer_can_rxSendThread(&links, &sync_data);
	HAL_Delay(100);

}
