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
    write_to_log_process("A new data.csv file has been created.");
        while (1) {
            if (sbuffer_remove(buffer, &data) == SBUFFER_SUCCESS ) {
                printf("find from store mgr: collecting data\n");
                printf("prewriting in the csv:%d, %lf, %ld\n", data.id, data.value, data.ts);
                if (data.id == 1) {
                    printf("finish collecting data in store mgr\n");
                    close_db(file);
                    printf("storemgr thread ended\n");
                    return 0;
                }
                fprintf(file,"%d, %lf, %ld\n", data.id, data.value, data.ts);
                fflush(file);
                write_to_log_process("Data insertion from sensor %d succeeded",data.id);
            }
            else {
                printf("failed to remove in store mgr\n");

            }
        }
    return 0;
    }


int close_db(FILE * f){
    if (f == NULL){
        return -1;
    }
    printf("closing csv\n");
    write_to_log_process("The data.csv file has been closed.");
    //end_log_process();
    return fclose(f);
}