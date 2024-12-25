//
// Created by fhr on 12/22/24.
//

#ifndef CONNMGR_H
#define CONNMGR_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include "sbuffer.h"
#include "config.h"

#include "lib/tcpsock.h"

typedef struct sbuffer sbuffer_t;

typedef struct serverArg serverArg_t;
typedef struct clientArg clientArg_t;

typedef struct serverArg {
    int PORT, MAX_CONN;
    sbuffer_t *buffer;
} serverArg_t;

typedef struct clientArg{
    tcpsock_t *client;
    sbuffer_t *buffer;
} clientArg_t;


void new_client(tcpsock_t *server,tcpsock_t *client);
int start_server(void* passArg);


#endif //CONNMGR_H
