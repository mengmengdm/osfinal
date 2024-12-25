//
// Created by fhr on 12/22/24.
//
#ifndef _SENSOR_DB_H_
#define _SENSOR_DB_H_

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
//#include "logger.h"
#include <stdbool.h>
#include "sbuffer.h"


typedef struct storeArg storeArg_t;
typedef struct storeArg {
    sbuffer_t *buffer;
} storeArg_t;


FILE * open_db(char * filename);
int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts);
int close_db(FILE * f);
void *storemgr(void *arg);

#endif /* _SENSOR_DB_H_ */
