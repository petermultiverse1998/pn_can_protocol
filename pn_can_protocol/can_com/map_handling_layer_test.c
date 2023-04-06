//
// Created by peter on 2/5/2023.
//
#include <malloc.h>
#include "stdio.h"
#include "map_handling_layer.h"
#include "sync_layer_can.h"
#include "base_layer_can.h"
#include "map.h"

SyncLayerCanLink link = {1, 2, 3, 4, 5};

#define LINK_ID 1

/////////////////////////SIMULATOR//////////////////////////////
static void can_receive(uint32_t id, uint8_t *bytes, uint8_t len) {

    //Transmitter
    can_com_receive_tx(LINK_ID, id, bytes, len);

    //Receiver
    can_com_receive_rx(LINK_ID, id, bytes, len);

}

static uint8_t can_send(uint32_t id, uint8_t *bytes, uint8_t len) {
    return base_layer_can_receive(id,bytes,len);;
}

static uint32_t getTimeInMillis() {
    return 1;
}

//////////////////////////TESTING/////////////////////////
static void dataReceivedCallback(uint32_t link_id,uint32_t id, uint16_t size) {
    printf("Received-> 0x%02x : ", (int)id);
    uint8_t bytes[size];
    can_com_getBytes_rx(link_id,id, bytes);
    for (int i = 0; i < size; ++i)
        printf("%d ", bytes[i]);
    printf("\n");
}


void map_handling_layer_test() {
    uint8_t data_1[5] = {1, 2, 3, 4, 5};
    uint8_t data_2[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    //Base Layer
    base_layer_can_init(can_send,can_receive,printf);

    //Initiate transmit and receive
    can_com_init_tx(getTimeInMillis, base_layer_can_send,printf,1);
    can_com_addLink_tx(LINK_ID,&link);

    can_com_init_rx(getTimeInMillis, base_layer_can_send, dataReceivedCallback,printf);
    can_com_addLink_rx(LINK_ID,&link);

    //Data transmit
    can_com_addMessage_tx(LINK_ID, 0xA, data_1, 5, 1);
    can_com_addMessage_tx(LINK_ID, 0xB, data_2, 11, 0);

    printf("Loop Begin : \n");
    for (int i = 0; i < 100; i++) {
        //transmit monitor
        base_layer_can_send_monitor();

        //receive monitor
        base_layer_can_receive_monitor();

        //map layer
        can_com_timeoutMonitor_tx(LINK_ID);
        can_com_timeoutMonitor_rx(LINK_ID);
    }
}
