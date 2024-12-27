//
// Created by fhr on 12/22/24.
//
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "datamgr.h"

#include <string.h>
#include <unistd.h>

#include "lib/dplist.h"

void *element_copy(void *element);
void element_free(void **element);
int element_compare(void *x, void *y);

sensor_element_t sensor_node[8];

void init_sensor_list() {
    sensor_node[0].sensor_id = 15;
    sensor_node[1].sensor_id = 21;
    sensor_node[2].sensor_id = 37;
    sensor_node[3].sensor_id = 49;
    sensor_node[4].sensor_id = 112;
    sensor_node[5].sensor_id = 129;
    sensor_node[6].sensor_id = 132;
    sensor_node[7].sensor_id = 142;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < RUN_AVG_LENGTH; j++) {
            sensor_node[i].averagedatalist[j] = 0;
        }
        sensor_node[i].runing_avg = 0;
    }
}

void calculate_allavg() {
    if (sensor_node[0].sensor_id != 15) {
        printf("DATAMGR: failed to initialize the sensor node\n");
    }
    for (int i = 0; i < 8; i++) {
        sensor_value_t sum = 0.0;
        for (int j = 0; j < RUN_AVG_LENGTH; j++) {sum = sum +sensor_node[i].averagedatalist[j];}
        sensor_node[i].runing_avg = sum/RUN_AVG_LENGTH;
    }
}

void calculate1avg(sensor_element_t *element) {
    sensor_value_t sum = 0;
    for (int j = 0; j < RUN_AVG_LENGTH; j++) {sum = sum +element->averagedatalist[j];}
    element->runing_avg = sum/RUN_AVG_LENGTH;

    check1avg(element);
}

void check_allavg() {
    for (int i = 0; i < 8; i++) {
        if (sensor_node[i].runing_avg != 0 && sensor_node[i].runing_avg > SET_MAX_TEMP) {
            printf("find sensor node %d over max_temp with avg %f\n",sensor_node[i].sensor_id,sensor_node[i].runing_avg);
        }
        else if (sensor_node[i].runing_avg != 0 && sensor_node[i].runing_avg < SET_MIN_TEMP) {
            printf("find sensor node %d less than min_temp with avg %f\n",sensor_node[i].sensor_id,sensor_node[i].runing_avg);
        }
    }
}
void check1avg(sensor_element_t *element) {
    if (element->runing_avg != 0 && element->runing_avg > SET_MAX_TEMP) {
        printf("find sensor node %d over max_temp with avg %f\n",element->sensor_id,element->runing_avg);
        write_to_log_process("Sensor node %d reports it’s too hot (avg temp = %f)",element->sensor_id,element->runing_avg);
    }
    else if (element->runing_avg != 0 && element->runing_avg < SET_MIN_TEMP) {
        printf("find sensor node %d less than min_temp with avg %f\n",element->sensor_id,element->runing_avg);
        write_to_log_process("Sensor node %d reports it’s too cold (avg temp = %f)",element->sensor_id,element->runing_avg);
    }
}
int insert_data(sbuffer_t *buffer) {
    sensor_data_t data;
    sensor_element_t *edit_element = NULL;
    if (sensor_node[0].sensor_id != 15) {
        printf("DATAMGR: failed to initialize the sensor node\n");
        return -1;
    }
    while (1) {
       if (buffer->head==NULL) {
           return -1;
       }

       if (buffer->head->data.flag == THREAD_READYTOREAD &&buffer->head->data.id != 1) {
           sbuffer_read(buffer, &data);
       }
       else if (buffer->head->data.id == 1) {
           sbuffer_read(buffer, &data);
           return -2;
       }
       else continue;

        sensor_id_t id = data.id;
        sensor_value_t value = data.value;
        printf("DATAMGR: reading the data from buffer %d, %f\n", id, value);
        //finding the corresponding sensor node
        for (int i = 0; i < 8; i++) {
            if (sensor_node[i].sensor_id == id) {
                edit_element = &sensor_node[i];
                break;
            }
        }
        if (edit_element == NULL) {
            printf("DATAMGR: failed to find the corresponding sensor node\n");
            if (id!=1){write_to_log_process("Received sensor data with invalid sensor node ID %d",id);}
            return -1;
        }

        //insert the value into the list to calculate average
        for (int i = 0; i < RUN_AVG_LENGTH; i++) {
            if (edit_element->averagedatalist[i] == 0) {
                edit_element->averagedatalist[i] = value;
                //calculate1avg(edit_element);
                return 0;
            }
        }

        for (int i = 0; i < RUN_AVG_LENGTH-1; i++) {
            edit_element->averagedatalist[i] = edit_element->averagedatalist[i+1];
        }
        edit_element->averagedatalist[RUN_AVG_LENGTH-1] = value;
        calculate1avg(edit_element);
        return 0;
    }
    }

void *datamgr(void *arg) {
    printf("DATAMGR: starting thread: datamgr\n");
    dataArg_t *data_arg = (dataArg_t*)arg;
    sbuffer_t *buffer = data_arg->buffer;
    //sensor_data_t *data;

    init_sensor_list();
    int flag = 0;
    while (buffer!=NULL) {
        flag =  insert_data(buffer);
        if (flag == -2){
            printf("datamgr thread end\n");
            return 0;}
    }
    return 0;
}


void datamgr_free(){}

