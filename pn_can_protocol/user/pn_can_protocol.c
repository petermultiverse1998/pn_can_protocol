/*
 * pn_can_protocol.c
 *
 *  Created on: Apr 6, 2023
 *      Author: peter
 */

#include "pn_can_protocol.h"
#include "sync_layer_can.h"
#include "stdarg.h"
#include "map.h"

#define PN_CAN_PROTOCOL_LINK_MAX_SIZE 10

static SyncLayerCanLink links[PN_CAN_PROTOCOL_LINK_MAX_SIZE];
static uint8_t link_size = 0;

static Map* map[PN_CAN_PROTOCOL_LINK_MAX_SIZE];

/*****************************CONSOLE*****************************\*/
typedef enum {
	CONSOLE_ERROR, CONSOLE_INFO, CONSOLE_WARNING
} ConsoleStatus;
static void console(ConsoleStatus status, const char *func_name,
		const char *msg, ...) {
//	if (status == CONSOLE_INFO)
//		return;
	//TODO make naked and show all registers
	if (status == CONSOLE_ERROR) {
		printf("pn_can_protocol.c|%s> ERROR :", func_name);
	} else if (status == CONSOLE_INFO) {
		printf("pn_can_protocol.c|%s> INFO : ", func_name);
	} else if (status == CONSOLE_WARNING) {
		printf("pn_can_protocol.c|%s> WARNING : ", func_name);
	} else {
		printf("pn_can_protocol.c|%s: ", func_name);
	}
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
}



uint8_t (*can_send_func)(uint32_t id, uint8_t *bytes, uint8_t len);
void pn_can_protocol_addLink(SyncLayerCanLink *link) {
	if (link == NULL) {
		console(CONSOLE_ERROR, __func__, "Link is NULL\n");
		return;
	}
	if (link_size >= PN_CAN_PROTOCOL_LINK_MAX_SIZE) {
		console(CONSOLE_ERROR, __func__, "Max size reached %d\n",
		PN_CAN_PROTOCOL_LINK_MAX_SIZE);
		return;
	}

	map[link_size] = map_create();
	if(map[link_size]==NULL){
		console(CONSOLE_ERROR, __func__, "Heap full\n");
		return;
	}


	links[link_size] = *link;
	console(CONSOLE_INFO, __func__, "Link 0x%0x is added in index %d\n", link,
			link_size);

	link_size++;
}

void pn_can_protocol_send(SyncLayerCanLink *link, uint8_t *data, uint16_t size) {

}

void pn_can_protocol_loop() {

}
