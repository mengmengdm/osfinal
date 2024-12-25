//
// Created by fhr on 12/22/24.
//
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "datamgr.h"

#include "lib/dplist.h"

void *element_copy(void *element);
void element_free(void **element);
int element_compare(void *x, void *y);

#define RUN_AVG_LENGTH 5

typedef struct
{
    room_id_t room_id;
    sensor_id_t sensor_id;
    sensor_value_t last_five_data[RUN_AVG_LENGTH];
    int size;
    sensor_value_t runing_avg;
    sensor_ts_t last_modified;
} my_element_t;


sensor_value_t insert_data(sensor_value_t arr[RUN_AVG_LENGTH], int size, sensor_value_t value) {
    if (size < RUN_AVG_LENGTH) {
        // insert into the array if it is not full
        arr[size] = value;
        (size)++;
    } else {
        // the array is full, delete the first element
        for (int i = 1; i < RUN_AVG_LENGTH; i++) {
            arr[i - 1] = arr[i];
        }
        arr[RUN_AVG_LENGTH - 1] = value;  // insert it to the end
    }

    sensor_value_t sum = 0;
    for (int i = 0; i < size; i++) {
        sum = sum + arr[i];
    }
    return sum/size;
}

void *element_copy(void *element)
{
    my_element_t *copy = malloc(sizeof(my_element_t));
    assert(copy != NULL);
    copy->room_id = ((my_element_t *)element)->room_id;
    copy->sensor_id = ((my_element_t *)element)->sensor_id;
    copy->runing_avg = ((my_element_t *)element)->runing_avg;
    copy->last_modified = ((my_element_t *)element)->last_modified;

    for (int i = 0; i < 5; i++) {
        copy->last_five_data[i] = ((my_element_t *)element)->last_five_data[i];
    }

    return (void *)copy;
}

void element_free(void **element)
{
    free(*element);
    *element = NULL;
}

int element_compare(void *x, void *y)
{
    my_element_t *elem1 = (my_element_t *)x;
    my_element_t *elem2 = (my_element_t *)y;
    if (elem1->room_id == elem2->room_id && elem1->sensor_id == elem2->sensor_id && elem1->runing_avg == elem2->runing_avg && elem1->last_modified == elem2->last_modified) {
        return 0;
    }
    return -1;
}

void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data){
    if (fp_sensor_map == NULL) {
        perror("Error opening fp_sensor_map file");
        return;
    }

    if (fp_sensor_data == NULL) {
        perror("Error opening fp_sensor_data file");
        return;
    }

    //create data_list to store all data
    dplist_t *data_list = dpl_create(element_copy, element_free,element_compare);

    room_id_t current_room_id = 0;
    sensor_id_t current_sensor_id = 0;
    while (fscanf(fp_sensor_map, "%d %d", &current_room_id, &current_sensor_id) == 2) {
        my_element_t * current_element = (my_element_t *)malloc(sizeof(my_element_t));
        current_element->room_id = current_room_id;
        current_element->sensor_id = current_sensor_id;
        current_element->size = 0;
        dpl_insert_at_index(data_list,current_element, -1,false);
        //printf("%d, %d\n", current_element->room_id,current_element->sensor_id);
    }

    sensor_id_t current_id_of_data = 0;
    sensor_value_t current_data = 0;
    time_t current_timestamp = 0;
    int data_list_size = dpl_size(data_list);

    while ( fread(&current_id_of_data, sizeof(sensor_id_t), 1, fp_sensor_data) == 1 &&
            fread(&current_data, sizeof(sensor_value_t), 1, fp_sensor_data) == 1 &&
            fread(&current_timestamp, sizeof(time_t), 1, fp_sensor_data) == 1) {
        my_element_t *find_element;
        for (int i = 0; i < data_list_size; i++) {
            find_element = dpl_get_element_at_index(data_list, i);
            if (current_id_of_data == find_element->sensor_id) {
                break;
            }
        }
        if (find_element != NULL) {
            if (find_element->size > 5) {
                find_element->size = 5;
            }
            find_element->runing_avg = insert_data(find_element->last_five_data, find_element->size, current_data);
            find_element->last_modified = current_timestamp;
            find_element->size = (find_element->size) + 1;
            printf("%u, %u, %f, %lld\n", find_element->room_id,find_element->sensor_id, find_element->runing_avg,find_element->last_modified);
        }

    }



}

void datamgr_free(){

}

uint16_t datamgr_get_room_id(sensor_id_t sensor_id){
    int data_list_size = dpl_size(data_list);

}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id){

}

time_t datamgr_get_last_modified(sensor_id_t sensor_id){

}

int datamgr_get_total_sensors(){

}