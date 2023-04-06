//
// Created by peter on 2/5/2023.
//

#include <malloc.h>
#include "map_handling_layer.h"
#include "map.h"

#define RECEIVING_TIMEOUT 100000   //in millisecond (ACK receive)

static Map *links_map;
static Map *waiting_maps;       //map with map content
static uint32_t memory_leak_track;
static Map *error_maps;
static int(*debugFunc)(const char *__restrict, ...) = NULL;
static uint8_t use_que = 0;

static void debug(const char func_name[], const char title[], const char msg[]) {
    if(debugFunc!=NULL)
        debugFunc("MAP LAYER TX|%s->%s: %s\n", func_name, title, msg);
}

static void dataTransmitted(SyncLayerCanLink *link, SyncLayerCanData *data, SyncLayerCanTransmitStatus status) {
    if(link==NULL){
        debug(__FUNCTION__, "LINK", "NULL");
        return;
    }

    if(waiting_maps == NULL) {
        debug(__FUNCTION__, "WAITING MAPS", "NULL");
        return;
    }
    if(error_maps == NULL) {
        debug(__FUNCTION__, "ERROR MAPS", "NULL");
        return;
    }

    uint32_t link_id = map_getKey(links_map, link);
    if(link_id == 0) {
        debug(__FUNCTION__, "LINK ID", "0(Doesn't exist)");
        return;
    }
    Map* waiting_map = map_get(waiting_maps,link_id);
    if(waiting_map == NULL) {
        debug(__FUNCTION__, "WAITING MAP", "NULL");
        return;
    }
    Map* error_map = map_get(error_maps,link_id);
    if(error_map == NULL) {
        debug(__FUNCTION__, "ERROR MAP", "NULL");
        return;
    }
    if(status==SYNC_LAYER_CAN_TRANSMIT_OK){
        if(data!=NULL){
            int size = sizeof(SyncLayerCanData);
            map_remove(waiting_map,data->id);//remove from waiting map
            free(data->bytes);
            free(data);
            memory_leak_track-=size;
        }else{
            debug(__FUNCTION__, "DATA", "NULL");
        }
    }else{
        //error
        if(map_get(error_map,data->id)!=NULL)
            return;//In case ID already exist
        map_remove(waiting_map,data->id);
        if(map_put(error_map,data->id,data)==NULL){
            debug(__FUNCTION__, "ERROR MAP PUT", "NULL(Heap is full)");
        }
    }
}

/**
 * This initiates map handling layer of transmits
 * @param timeInMillisFunc  : this function gives time in millisecond
 * @param canSendFunc       : This function send the can
 * @param printf            : For debug (Can be null)
 * @param use_queue         : force to use que instead of map
 */
void can_com_init_tx(uint32_t (*timeInMillisFunc)(),
                     uint8_t (*canSendFunc)(uint32_t,uint8_t*, uint8_t),
                     int(*printf)(const char *__restrict, ...),uint8_t use_queue){
    use_que = use_queue;
    debugFunc = printf;
    sync_layer_can_tx_init(timeInMillisFunc,canSendFunc,dataTransmitted);
    links_map = map_create();
    if(links_map==NULL){
        debug(__FUNCTION__, "LINKS MAP CREATE", "NULL(Heap is full)");
    }
    waiting_maps = map_create();
    if(waiting_maps==NULL){
        debug(__FUNCTION__, "WAITING MAPS CREATE", "NULL(Heap is full)");
    }
    error_maps = map_create();
    if(error_maps==NULL){
        debug(__FUNCTION__, "ERROR MAPS CREATE", "NULL(Heap is full)");
    }
    memory_leak_track = 0;
}

/**
 * This will add link in links with corresponding ID
 * @param link_id    : Id to be designated to link
 * @param link  : Link
 */
void can_com_addLink_tx(uint32_t link_id,SyncLayerCanLink *link){
    if(waiting_maps == NULL) {
        debug(__FUNCTION__, "WAITING MAPS", "NULL");
        return;
    }
    if(error_maps == NULL) {
        debug(__FUNCTION__, "ERROR MAPS", "NULL");
        return;
    }
    if(links_map == NULL) {
        debug(__FUNCTION__, "LINKS MAP", "NULL");
        return;
    }
    if(map_put(links_map,link_id,link) == NULL) {
        debug(__FUNCTION__, "LINKS MAP PUT", "NULL(Heap  is full)");
        return;
    }
    Map* waiting_map = map_create();
    if(waiting_map == NULL) {
        debug(__FUNCTION__, "WAITING MAP CREATE", "NULL(Heap  is full)");
        return;
    }
    Map* error_map = map_create();
    if(error_map == NULL) {
        debug(__FUNCTION__, "ERROR MAP CREATE", "NULL(Heap  is full)");
        return;
    }
    if(map_put(waiting_maps,link_id,waiting_map) == NULL) {
        debug(__FUNCTION__, "WAITING MAPS PUT", "NULL(Heap  is full)");
        return;
    }
    if(map_put(error_maps,link_id,error_map) == NULL) {
        debug(__FUNCTION__, "ERROR MAPS PUT", "NULL(Heap  is full)");
        return;
    }
}

/**
 * This add the message pointer to waiting map
 * @param link_id    : Id to be designated to link
 * @param id            : CAN ID of message
 * @param bytes         : bytes ot be sent
 * @param len           : Length of message or size in bytes
 * @param one_shot      : 1 for data send in one shot or 0 for each data send require ack
 */
void can_com_addMessage_tx(uint32_t link_id,uint32_t id, uint8_t* bytes, uint16_t len, uint8_t one_shot){
    if(waiting_maps == NULL) {
        debug(__FUNCTION__, "WAITING MAPS", "NULL");
        return;
    }
    if(error_maps == NULL) {
        debug(__FUNCTION__, "ERROR MAPS", "NULL");
        return;
    }
    Map* waiting_map = map_get(waiting_maps,link_id);
    if(waiting_map == NULL) {
        debug(__FUNCTION__, "WAITING MAP", "NULL");
        return;
    }
    Map* error_map = map_get(error_maps,link_id);
    if(error_map == NULL) {
        debug(__FUNCTION__, "ERROR MAP", "NULL");
        return;
    }
    if(map_get(waiting_map,id)!=NULL)
        return;//In case of ID already exist in waiting map
    if(map_get(error_map,id)!=NULL)
        return;//In case of ID already exist in error map
    SyncLayerCanLink *link = map_get(links_map,link_id);
    if(link == NULL) {
        debug(__FUNCTION__, "LINK", "NULL");
        return;
    }

    uint32_t size = sizeof(SyncLayerCanData);
    SyncLayerCanData *data = (SyncLayerCanData*)malloc(size);
    if(data == NULL) {
        debug(__FUNCTION__, "DATA MALLOC", "NULL(Heap is full)");
        return;
    }
    memory_leak_track+=size;

    data->id = id;
    data->bytes = (uint8_t*) malloc(len);
    if(data->bytes == NULL) {
        debug(__FUNCTION__, "BYTES MALLOC", "NULL(Heap is full)");
        return;
    }
    for (int i = 0; i < len; ++i)
        data->bytes[i] = bytes[i];
    data->size = len;
    data->oneShot = one_shot;
    data->time_elapse = 0;//Data is not sent yet
    if(map_put(waiting_map,id,data)==NULL){
        debug(__FUNCTION__, "WAITING MAP PUT", "NULL(Heap is full)");
    }
    sync_layer_can_tx_send(link,data);
}

/**
 * This function should be called everytime something received in CAN
 * @param link_id        : Destination id where message is to be sent
 * @param id            : CAN ID of message
 * @param bytes         : bytes to be sent
 * @param len           : Length of message
 */
void can_com_receive_tx(uint32_t link_id, uint32_t id, uint8_t *bytes, uint8_t len){
    if(waiting_maps == NULL) {
        debug(__FUNCTION__, "WAITING MAPS", "NULL");
        return;
    }
    if(error_maps == NULL) {
        debug(__FUNCTION__, "ERROR MAPS", "NULL");
        return;
    }
    Map* waiting_map = map_get(waiting_maps,link_id);
    if(waiting_map == NULL) {
        debug(__FUNCTION__, "WAITING MAP", "NULL");
        return;
    }
    Map* error_map = map_get(error_maps,link_id);
    if(error_map == NULL) {
        debug(__FUNCTION__, "ERROR MAP", "NULL");
        return;
    }
    SyncLayerCanLink *link = map_get(links_map,link_id);
    if(link == NULL) {
        debug(__FUNCTION__, "LINK", "NULL");
        return;
    }

    uint32_t data_id = sync_layer_can_tx_recDataId(link,id,bytes);
    if(data_id>0){
        SyncLayerCanData* data = map_get(waiting_map,data_id);
        if(data!=NULL){
            sync_layer_can_tx_rec(link,data,id,bytes,len);
        }
    }
}

/**
 * This function should be called in timer to monitor timeout
 * @param link_id        : Destination id where message is to be sent
 */
void can_com_timeoutMonitor_tx(uint32_t link_id){
    if(waiting_maps == NULL) {
        debug(__FUNCTION__, "WAITING MAPS", "NULL");
        return;
    }
    if(error_maps == NULL) {
        debug(__FUNCTION__, "ERROR MAPS", "NULL");
        return;
    }
    Map* waiting_map = map_get(waiting_maps,link_id);
    if(waiting_map == NULL) {
        debug(__FUNCTION__, "WAITING MAP", "NULL");
        return;
    }
    Map* error_map = map_get(error_maps,link_id);
    if(error_map == NULL) {
        debug(__FUNCTION__, "ERROR MAP", "NULL");
        return;
    }

    //Check for timeout for waiting map
    int size = waiting_map->size;
    uint32_t ids[size];
    map_getKeys(waiting_map,ids);
    for(int i=0;i<size;i++){
        SyncLayerCanData *data = map_get(waiting_map,ids[i]);
        if(data==NULL)
            continue;
        if(data->time_elapse < RECEIVING_TIMEOUT)
            continue;
        map_remove(waiting_map,ids[i]);
        map_put(error_map,ids[i],data);
    }
}

/**
 * It retries the failed data
 * @param link_id        : Destination id where message is to be sent
 */
void can_com_errorHandler_tx(uint32_t link_id){
    if(error_maps == NULL) {
        debug(__FUNCTION__, "ERROR MAPS", "NULL");
        return;
    }
    Map* error_map = map_get(error_maps,link_id);
    if(error_map == NULL) {
        debug(__FUNCTION__, "ERROR MAP", "NULL");
        return;
    }
    SyncLayerCanLink *link = map_get(links_map,link_id);
    if(link == NULL) {
        debug(__FUNCTION__, "LINK", "NULL");
        return;
    }

    //Retry
    int size = error_map->size;
    uint32_t ids[size];
    map_getKeys(error_map,ids);
    for(int i=0;i<size;i++){
        SyncLayerCanData *data = map_get(error_map,ids[i]);
        sync_layer_can_tx_send(link,data);
    }
}
