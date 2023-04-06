//
// Created by peter on 2/5/2023.
//

#include <malloc.h>
#include "map_handling_layer.h"
#include "map.h"

#define RECEIVING_TIMEOUT 100000   //in millisecond

static Map *links_map;
static Map *waiting_maps;
static uint32_t memory_leak_track;
static void (*data_received_callback)(uint32_t,uint32_t,uint16_t);
static int(*debugFunc)(const char *__restrict, ...) = NULL;

static void debug(const char func_name[], const char title[], const char msg[]) {
    if(debugFunc!=NULL)
        debugFunc("MAP LAYER RX|%s->%s: %s\n", func_name, title, msg);
}

static void dataReceivedCallback(uint32_t link_id,uint32_t id,uint16_t size){
    if(data_received_callback!=NULL)
        data_received_callback(link_id,id,size);
}

static void dataReceived(SyncLayerCanLink *link, SyncLayerCanData *data, SyncLayerCanReceiveStatus status) {
    if(status==SYNC_LAYER_CAN_RECEIVE_OK){

    }
}

/**
 * This initiates map handling layer of receives
 * @param timeInMillisFunc          : this function gives time in millisecond
 * @param canSendFunc               : This function send the can
 * @param dataReceivedCallbackFunc  : This function is called each time
 * @param printf                    : For debug (Can be null)
 */
void can_com_init_rx(uint32_t (*timeInMillisFunc)(),
                     uint8_t (*canSendFunc)(uint32_t,uint8_t*, uint8_t),
                     void (*dataReceivedCallbackFunc)(uint32_t ,uint32_t,uint16_t),
                     int(*printf)(const char *__restrict, ...)){
    debugFunc = printf;
    data_received_callback = dataReceivedCallbackFunc;
    sync_layer_can_rx_init(timeInMillisFunc,canSendFunc,dataReceived);
    links_map = map_create();
    if(links_map == NULL) {
        debug(__FUNCTION__, "LINKS MAP", "NULL(Heap is full)");
        return;
    }
    waiting_maps = map_create();
    if(waiting_maps == NULL) {
        debug(__FUNCTION__, "WAITING MAPS", "NULL(Heap is full)");
        return;
    }
    memory_leak_track = 0;
}

/**
 * This will add link in links with corresponding ID
 * @param link_id    : Id to be designated to link
 * @param link  : Link
 */
void can_com_addLink_rx(uint32_t link_id,SyncLayerCanLink *link){
    if(links_map == NULL) {
        debug(__FUNCTION__, "LINKS MAP", "NULL");
        return;
    }
    if(waiting_maps == NULL) {
        debug(__FUNCTION__, "WAITING MAPS", "NULL");
        return;
    }
    if(map_put(links_map,link_id,link)==NULL){
        debug(__FUNCTION__, "LINKS MAP PUT", "NULL(Heap is full)");
        return;
    }
    Map* waiting_map = map_create();
    if(waiting_map==NULL){
        debug(__FUNCTION__, "WAITING MAP CREATE", "NULL(Heap is full)");
        return;
    }
    if(map_put(waiting_maps,link_id,waiting_map)==NULL)
        debug(__FUNCTION__, "WAITING MAP PUT", "NULL(Heap is full)");
}

/**
 * This function should be called everytime something received in CAN
 * @param link_id    : Id to be designated to link
 * @param id            : CAN ID of message
 * @param bytes         : bytes received
 * @param len           : Length of message
 */
void can_com_receive_rx(uint32_t link_id, uint32_t id, uint8_t *bytes, uint8_t len){
    if(waiting_maps == NULL) {
        debug(__FUNCTION__, "WAITING MAPS", "NULL");
        return;
    }
    Map* waiting_map = map_get(waiting_maps,link_id);
    if(waiting_map == NULL) {
        debug(__FUNCTION__, "WAITING MAP", "NULL");
        return;
    }
    SyncLayerCanLink *link = map_get(links_map,link_id);
    if(link == NULL) {
        debug(__FUNCTION__, "LINK", "NULL");
        return;
    }

    uint32_t data_id = sync_layer_can_rx_recDataId(link,id,bytes);
    SyncLayerCanData* data = map_get(waiting_map, data_id==0?id:data_id);
    if(data_id>0 || data!=NULL){
        if(data==NULL){
            //If data_id doesn't exist
            int size = sizeof(SyncLayerCanData);
            data = (SyncLayerCanData*)malloc(size);
            if(data!=NULL) {
                memory_leak_track += size;
                map_put(waiting_map,data_id,data);
                sync_layer_can_rx_rec(link,data,id,bytes,len);
            }else{
                debug(__FUNCTION__, "DATA MALLOC", "NULL(Heap is full)");
            }
        }else{
            //If data_id already exist
            if(data!=NULL){
                sync_layer_can_rx_rec(link,data,id,bytes,len);
            }else{
                debug(__FUNCTION__, "DATA", "NULL");
            }
        }
    }
}

/**
 * This function should be called in timer to monitor timeout
 * This removes if timeout
 * @param link_id    : Id to be designated to link
 */
void can_com_timeoutMonitor_rx(uint32_t link_id){
    if(waiting_maps == NULL) {
        debug(__FUNCTION__, "WAITING MAPS", "NULL");
        return;
    }
    Map* waiting_map = map_get(waiting_maps,link_id);
    if(waiting_map == NULL) {
        debug(__FUNCTION__, "WAITING MAP", "NULL");
        return;
    }
    SyncLayerCanLink *link = map_get(links_map,link_id);
    if(link == NULL) {
        debug(__FUNCTION__, "LINK", "NULL");
        return;
    }

    //Check for timeout
    int size = waiting_map->size;
    uint32_t ids[size];
    map_getKeys(waiting_map,ids);
    for(int i=0;i<size;i++){
        SyncLayerCanData *data = map_get(waiting_map,ids[i]);
        if(data==NULL)
            continue;
        if(data->time_elapse < RECEIVING_TIMEOUT){
            if(data->count==data->size && data->time_elapse == 0)
                dataReceivedCallback(link_id,data->id,data->size);
            continue;
        }
        map_remove(waiting_map,ids[i]);//remove if timeout
        free(data);
        memory_leak_track-=sizeof(SyncLayerCanData);
    }
}

/**
 * This gets bytes
 * @param link_id    : Id to be designated to link
 * @param id    : id of data to be received
 * @param bytes : bytes to be received
 */
void can_com_getBytes_rx(uint32_t link_id,uint32_t id,uint8_t*bytes){
    if(waiting_maps == NULL) {
        debug(__FUNCTION__, "WAITING MAPS", "NULL");
        return;
    }
    Map* waiting_map = map_get(waiting_maps,link_id);
    if(waiting_map == NULL) {
        debug(__FUNCTION__, "WAITING MAP", "NULL");
        return;
    }

    SyncLayerCanData *data = map_get(waiting_map,id);
    if(data==NULL) {
        debug(__FUNCTION__, "DATA", "NULL");
        return;
    }
    for (int i = 0; i < data->size; ++i)
        bytes[i] = data->bytes[i];
    free(data->bytes);
    free(data);
    map_remove(waiting_map,id);
    memory_leak_track-= sizeof(SyncLayerCanData);
}

