//
// Created by fhr on 12/22/24.
//
//
// Created by fhr on 11/23/24.
//
#include "sensor_db.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

sbuffer_t *buffer;
//storage manager thread
FILE * open_db(char *filename){
    FILE * file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open the file");
        return NULL;
    }
    return file;
}

void *storemgr(void *arg) {
    printf("starting thread: storemgr\n");
    storeArg_t *store_arg = (storeArg_t *)arg;
    sensor_data_t data;
    buffer = store_arg->buffer;
    if (buffer == NULL) {
        printf("storemgr failed to fetch shared data");
    }
    FILE *file = open_db("data.csv");
        while (1) {
            if (sbuffer_remove(buffer, &data) == SBUFFER_SUCCESS && data.flag == 0) {
                printf("find from store mgr: collecting data\n");
                printf("writing in the csv:%d, %lf, %ld\n", data.id, data.value, data.ts);
                fprintf(file,"%d, %lf, %ld\n", data.id, data.value, data.ts);
            }
            else if (sbuffer_remove(buffer, &data) == SBUFFER_SUCCESS && data.flag == 1) {
                printf("finish collecting data in store mgr\n");
                close_db(file);
                break;
            }
            else {
                printf("failed to remove in store mgr\n");
                break;
            }
        }
    }


int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts){
    if (f == NULL){
        return -1;
    }
    write_to_log_process("Data inserted.");

    int result = fprintf(f, "%d, %.6f, %ld\n", id, value, ts);

    if (result < 0) {
        return result;
    }

    return 0;
}

int close_db(FILE * f){
    if (f == NULL){
        return -1;
    }
    printf("closing csv\n");
    write_to_log_process("Data file closed.");
    //end_log_process();
    return fclose(f);
}