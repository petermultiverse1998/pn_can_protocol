//
// Created by niruja on 1/31/2023.
//

#include <malloc.h>
#include "sync_layer_can.h"

static uint8_t (*rx_can_send)(uint32_t, uint8_t*, uint8_t);
static uint32_t (*time_in_millis)();
static void (*data_received)(SyncLayerCanLink*, SyncLayerCanData*, SyncLayerCanReceiveStatus);

static uint32_t timeInMillis(){
    if(time_in_millis==NULL)
        return 0;
    else
        return time_in_millis();
}

static void dataReceived(SyncLayerCanLink *link, SyncLayerCanData *data, SyncLayerCanReceiveStatus status) {
    if(data_received!=NULL)
        data_received(link,data,status);
}

/**
 * It initiates can receive
 * @param getTimeInMillisFunc : function that return time in millis
 * @param canSendFunc : can transmit function
 * @param dataReceivedFunc : this callback function is called after data is received with status
 */
void sync_layer_can_rx_init(uint32_t (*getTimeInMillisFunc)(),
                            uint8_t (*canSendFunc)(uint32_t id, uint8_t* bytes, uint8_t len),
                            void (*dataReceivedFunc)(SyncLayerCanLink*, SyncLayerCanData*, SyncLayerCanReceiveStatus)) {
    time_in_millis = getTimeInMillisFunc;
    rx_can_send = canSendFunc;
    data_received = dataReceivedFunc;
}

/**
 * This returns the received Data id
 * @param link   : link where data is to be received
 * @param id            : CAN id that is received
 * @param bytes         : received bytes from CAN
 * @return              : Received data id
 *                      : 0 for id not in link
 */
uint32_t sync_layer_can_rx_recDataId(SyncLayerCanLink* link, uint32_t id, uint8_t* bytes){
    if(id==link->start_req_ID//START REQ
       ||id==link->end_req_ID//END REQ
            )
        return *(uint32_t*)bytes;
    return 0;
}

/**
 * This should be called whenever CAN receive any new message
 * @param link   : link where data is to be received
 * @param data          : data to be transmitted
 * @param id            : CAN id that is received
 * @param bytes         : received bytes from CAN (!!!Allocated memory should be freed)
 * @param len           : received bytes size
 */
void sync_layer_can_rx_rec(SyncLayerCanLink* link, SyncLayerCanData *data, uint32_t id, uint8_t* bytes, uint8_t len) {
    if(id==link->start_req_ID){
        //START REQ
        data->id = *(uint32_t *) bytes;
        data->size = *(uint16_t *) ((uint32_t *) bytes + 1);
        data->oneShot = *(uint8_t*)((uint16_t *) ((uint32_t *) bytes + 1)+1);
        data->count = 0;
        data->bytes = malloc(data->size);
        //START ACK
        data->time_elapse = timeInMillis();
        uint8_t status = rx_can_send(link->start_ack_ID,bytes,8);
        if(!status)
            dataReceived(link,data,SYNC_LAYER_CAN_RECEIVE_START_ACK_FAILED);
    }else if(id==data->id){
        //DATA
        for(uint8_t i=0;i<len;i++)
            data->bytes[data->count++] = bytes[i];

        if(data->oneShot && data->count<data->size)
            return;

        //DATA ACK
//        printf("Okay\n");
        uint32_t crc = 0;
        uint8_t can_bytes[8];
        *(uint32_t*)can_bytes = data->id;
        *((uint32_t*)can_bytes+1) = crc;
        data->time_elapse = timeInMillis();
        uint8_t status = rx_can_send(link->data_ack_ID,can_bytes,8);
        if(!status)
            dataReceived(link,data,SYNC_LAYER_CAN_RECEIVE_DATA_ACK_FAILED);
    }else if(id==link->end_req_ID){
        //END REQ
        //END ACK
        uint32_t crc = 0;
        uint8_t can_bytes[8];
        *(uint32_t*)can_bytes = data->id;
        *((uint32_t*)can_bytes+1) = crc;
        data->time_elapse = 0;//data receive complete
        uint8_t status = rx_can_send(link->end_ack_ID,can_bytes,8);
        if(!status)
            dataReceived(link,data,SYNC_LAYER_CAN_RECEIVE_END_ACK_FAILED);

        //data received call back
        dataReceived(link,data,SYNC_LAYER_CAN_RECEIVE_OK);
    }
}
