/*
 * user.c
 *
 *  Created on: Apr 6, 2023
 *      Author: peter
 */

#include "user.h"
#include "pn_can_protocol.h"


static SyncLayerCanLink link = { 1, 2, 3, 4, 5, 6, 7 };

/********************CONSOLE***************************/
static void console(const char *title, const char *msg) {
	printf("%s:: %s\n", title, msg);
}

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
//	printf("Interrupt-> 0x%02x : ", (unsigned int) rx_header.ExtId);
//	for (int i = 0; i < rx_header.DLC; ++i)
//		printf("%d ", data[i]);
//	printf("\n");
	pn_can_protocol_recThread(&link, rx_header.ExtId, data, rx_header.DLC);
}

static CAN_TxHeaderTypeDef tx_header;
static uint32_t tx_mailbox;
static uint8_t canSend(uint32_t id, uint8_t *bytes, uint8_t len) {
	tx_header.DLC = len;
	tx_header.ExtId = id;
	tx_header.IDE = CAN_ID_EXT;
	tx_header.RTR = CAN_RTR_DATA;
	tx_header.TransmitGlobalTime = DISABLE;

//	printf("canSend-> 0x%02x : ", (unsigned int) id);
//	for (int i = 0; i < len; ++i)
//		printf("%d ", bytes[i]);
//	printf("\n");

	return HAL_CAN_AddTxMessage(&hcan, &tx_header, bytes, &tx_mailbox) == HAL_OK;
}

/**********************MAIN THREAD****************************/
static uint8_t txCallback(uint32_t id,uint8_t* bytes,uint16_t size,uint8_t status) {
	printf("Tx Data : ");
	if(!status){
		printf("failed\n");
		return 1;
	}
	printf("0x%0x -> ",id);
	for (int i = 0; i < size; i++)
		printf("%d ", bytes[i]);
	printf("\n");
	if(id>=0x20 && id<0x30)
		pn_can_protocol_addTxMessagePtr(&link, id+1, bytes,size);
	return 1;
}

static uint8_t rxCallback(uint32_t id,uint8_t* bytes,uint16_t size,uint8_t status) {
	printf("Rx Data : ");
	if(!status){
		printf("failed\n");
		return 1;
	}
	printf("0x%0x -> ",id);
	for (int i = 0; i < size; i++)
		printf("%d ", bytes[i]);
	printf("\n");

	return 1;
}

uint8_t tx_bytes[] = {1,2,3,4,[9]=10};
uint8_t rx_bytes[10];
void init() {
	canInit();
	console("\n\nSOURCE INIT", "SUCCESS");

	pn_can_protocol_addLink(&link, canSend, txCallback, rxCallback);

	pn_can_protocol_addTxMessagePtr(&link, 0x4FF, tx_bytes, sizeof(tx_bytes));
//	pn_can_protocol_addRxMessagePtr(&link, 0xA, rx_bytes, sizeof(rx_bytes));

	HAL_Delay(3000);
}

void loop() {
	pn_can_protocol_sendThread(&link);
//	HAL_Delay(1000);
}
