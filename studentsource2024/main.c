//
// Created by fhr on 12/22/24.
//
#include <stdio.h>
#include <stdlib.h>
#include "connmgr.h"
#include "sbuffer.h"
#include "sensor_db.h"



int write_to_log_process(char *msg) {
	return 0;
}

int main(int argc, char *argv[]){
	if(argc < 3){
		printf("Please provide the right arguments: first the port, then the max nb of clients");
		return -1;
	}
	sbuffer_t **shared_buffer = malloc(8);
	sbuffer_init(shared_buffer);

	int MAX_CONN = atoi(argv[2]);
	int PORT = atoi(argv[1]);
    printf("calling to start server on port: %d\n",PORT);

	serverArg_t *serverArg = (serverArg_t *)malloc(sizeof(serverArg_t));
	if (serverArg == NULL) {
		printf("Failed to allocate memory for serverArg");
		return -1;
	}
	serverArg->PORT = PORT;
	serverArg->MAX_CONN = MAX_CONN;
	serverArg->buffer = *shared_buffer;

	storeArg_t *storeArg = (storeArg_t *)malloc(sizeof(storeArg_t));
	if (storeArg == NULL) {
		printf("Failed to allocate memory for storeArg");
		return -1;
	}
	storeArg->buffer = *shared_buffer;

	// Create the manager threads
	pthread_t connmgr_thread, datamgr_thread, storagemgr_thread;
	pthread_create(&connmgr_thread, NULL, (void*)start_server, serverArg);
	//pthread_create(&datamgr_thread, NULL, (void*)datamgr, data_args);
	pthread_create(&storagemgr_thread, NULL, (void*)storemgr, storeArg);


	pthread_join(connmgr_thread, NULL);
	pthread_join(storagemgr_thread, NULL);

}
