//
// Created by niruja on 1/31/2023.
//

#include "sync_layer_can.h"

static uint8_t (*tx_can_send)(uint32_t,uint8_t*, uint8_t);
static uint32_t (*time_in_millis)();
static void (*data_transmitted)(SyncLayerCanLink*, SyncLayerCanData*, SyncLayerCanTransmitStatus);

static uint32_t timeInMillis(){
    if(time_in_millis==NULL)
        return 0;
    else
        return time_in_millis();
}

static void dataTransmitted(SyncLayerCanLink *link, SyncLayerCanData *data, SyncLayerCanTransmitStatus status) {
    if(data_transmitted!=NULL)
        data_transmitted(link,data,status);
}

/**
 * This initiate Sync Layer of CAN transmit
 * @param getTimeInMillisFunc       : this func returns time in millis
 * @param can_send                  : function that send CAN
 * @param dataTransmittedFunc       : this callback function is called after data is transmitted with status
 */
void sync_layer_can_tx_init(uint32_t (*getTimeInMillisFunc)(),
                            uint8_t (*canSendFunc)(uint32_t id, uint8_t* bytes, uint8_t len),
                            void (*dataTransmittedFunc)(SyncLayerCanLink*, SyncLayerCanData*, SyncLayerCanTransmitStatus)) {
    tx_can_send = canSendFunc;
    time_in_millis = getTimeInMillisFunc;
    data_transmitted = dataTransmittedFunc;
}

void sync_layer_can_tx_send(SyncLayerCanLink *link, SyncLayerCanData *data) {
    //START_REQ
    uint8_t bytes[8] = {0};
    *(uint32_t *) bytes = data->id;
    *(uint16_t *) ((uint32_t *) bytes + 1) = data->size;
    *(uint8_t*)((uint16_t *) ((uint32_t *) bytes + 1)+1) = data->oneShot;
    data->count = 0;
    data->time_elapse = timeInMillis();
    uint8_t status  = tx_can_send(link->start_req_ID, bytes, 8);
    if(!status)
        dataTransmitted(link,data,SYNC_LAYER_CAN_TRANSMIT_START_REQ_FAILED);
}

/**
 * This returns the received Data id
 * @param link   : link where data is to be transmitted
 * @param id            : CAN id that is received
 * @param bytes         : received bytes from CAN
 * @return              : Received data id
 *                      : 0 for id not in link
 */
uint32_t sync_layer_can_tx_recDataId(SyncLayerCanLink* link, uint32_t id, uint8_t* bytes){
    if(id==link->start_ack_ID//START ACK
       ||id==link->data_ack_ID//DATA ACK
       ||id==link->end_ack_ID//END ACK
            )
        return *(uint32_t*)bytes;
    return 0;
}

/**
 * This should be called whenever CAN receive any new message
 * @param link   : link where data is to be transmitted
 * @param data          : data to be transmitted
 * @param id            : CAN id that is received
 * @param bytes         : received bytes from CAN
 * @param len           : received bytes size
 */
void sync_layer_can_tx_rec(SyncLayerCanLink* link, SyncLayerCanData *data, uint32_t id, uint8_t* bytes, uint8_t len) {
    if(id==link->start_ack_ID){
        //START ACK
        uint16_t data_size = *(uint16_t*)((uint32_t*)bytes+1);
        if(data_size!=data->size){
            dataTransmitted(link,data,SYNC_LAYER_CAN_TRANSMIT_START_ACK_FAILED);
            return;
        }

        //DATA
        if(data->oneShot){
            while(1){
                if(data->count>=data->size)
                    break;

                uint8_t new_data_size = data->size-data->count;
                if(new_data_size>=8)
                    new_data_size = 8;

                uint16_t offset = data->count;
                data->count += new_data_size;
                data->time_elapse = timeInMillis();
                uint8_t status = tx_can_send(data->id,(data->bytes+offset),new_data_size);
                if(!status)
                    dataTransmitted(link,data,SYNC_LAYER_CAN_TRANSMIT_DATA_TRANSMIT_FAILED);
            }
            return;
        }

        uint8_t new_data_size = data->size-data->count;
        if(new_data_size>=8)
            new_data_size = 8;
        data->count += new_data_size;
        data->time_elapse = timeInMillis();
        uint8_t status = tx_can_send(data->id,data->bytes,new_data_size);
        if(!status)
            dataTransmitted(link,data,SYNC_LAYER_CAN_TRANSMIT_DATA_TRANSMIT_FAILED);
    }else if(id==link->data_ack_ID){
        //DATA ACK
        uint32_t crc = *((uint32_t*)bytes+1);

        //todo: CRC Check


        if(data->count < data->size){
            //DATA
            uint8_t new_data_size = data->size-data->count;
            if(new_data_size>=8)
                new_data_size = 8;

            uint16_t offset = data->count;
            data->count += new_data_size;
            data->time_elapse = timeInMillis();
            uint8_t status = tx_can_send(data->id,(data->bytes+offset),new_data_size);
            if(!status)
                dataTransmitted(link,data,SYNC_LAYER_CAN_TRANSMIT_DATA_TRANSMIT_FAILED);
            return;
        }

        //END REQ
        uint32_t CAN_id = link->end_req_ID;
        uint8_t CAN_bytes[8];
        *(uint32_t*)CAN_bytes = data->id;
        data->time_elapse = timeInMillis();
        uint8_t status = tx_can_send(CAN_id,CAN_bytes,8);
        if(!status)
            dataTransmitted(link,data,SYNC_LAYER_CAN_TRANSMIT_END_REQ_FAILED);
    }else if(id==link->end_ack_ID){
        //END ACK
        //todo: resend check

        //data transmitted callback
        dataTransmitted(link,data,SYNC_LAYER_CAN_TRANSMIT_OK);
    }

}
