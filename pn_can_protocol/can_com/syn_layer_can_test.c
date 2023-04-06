//
// Created by niruja on 1/31/2023.
//

#include <stdio.h>
#include <malloc.h>
#include "sync_layer_can.h"

/////////////////////////SIMULATOR///////////////////////////////////
static void can_receive(uint32_t, uint8_t*, uint8_t);
static uint8_t can_send(uint32_t id, uint8_t* bytes, uint8_t len) {
    uint8_t can_data[8];
    for(int i=0;i<len;i++)
        can_data[i]=bytes[i];
    can_receive(id, can_data, len);
    return 1;
}

static uint32_t getTimeInMillis() {
    return 10000;
}

//////////////////////////////////MAIN_TEST////////////////////////////////////////
static void dataTransmitted(SyncLayerCanLink *destination, SyncLayerCanData *data, SyncLayerCanTransmitStatus status) {
    if(status==SYNC_LAYER_CAN_TRANSMIT_OK){
        printf("Transmitted Successfully\n");

        printf("0x%02x : ",data->id);
        for (int i = 0; i < data->size; ++i)
            printf("%d ",data->bytes[i]);
        printf("\n");
    }
}


static void dataReceived(SyncLayerCanLink *destination, SyncLayerCanData *data, SyncLayerCanReceiveStatus status) {
    if(status==SYNC_LAYER_CAN_RECEIVE_OK){
        printf("Received Successfully\n");
        printf("0x%02x : ",data->id);
        for (int i = 0; i < data->size; ++i)
            printf("%d ",data->bytes[i]);
        printf("\n");
    }
}



static uint8_t b[] = {1, 2, 3, 4, 5,6,7,8,9,10,11,12,13,14,15,16,17};
SyncLayerCanLink tx_destination = {1, 2, 3, 4, 5};
SyncLayerCanData tx_data = {0xA,b, 17,1};

SyncLayerCanLink rx_destination = {1, 2, 3, 4, 5};
SyncLayerCanData rx_data;

static void can_receive(uint32_t id, uint8_t* bytes, uint8_t len) {
    //For transmitting
    uint32_t data_tx_ID = sync_layer_can_tx_recDataId(&tx_destination,id,bytes);
    if(data_tx_ID>0){
        sync_layer_can_tx_rec(&tx_destination,&tx_data,id,bytes,len);
    }

    //For receiving
    uint32_t data_rx_ID = sync_layer_can_rx_recDataId(&rx_destination,id,bytes);
    if(data_rx_ID>0||id==rx_data.id){
        if(data_rx_ID>0)
            rx_data.id = data_rx_ID;
        sync_layer_can_rx_rec(&rx_destination,&rx_data,id,bytes,len);
    }
}
void sync_layer_can_test() {
    //Transmit and Receive Init
    sync_layer_can_tx_init(getTimeInMillis,can_send,dataTransmitted);
    sync_layer_can_rx_init(getTimeInMillis,can_send,dataReceived);

    //Transmit data
    sync_layer_can_tx_send(&tx_destination, &tx_data);

}