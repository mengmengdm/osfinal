//
// Created by fhr on 12/22/24.
//
#include "connmgr.h"


/**
 * Implements a sequential test server (only one connection at the same time)
 */
//format: ./sensor_gateway 1234 3 starts a server with a connection manager listening on port 1234.
//

int MAX_CONN = 0;
int PORT = 0;
int start_thread, end_thread = 0;
tcpsock_t *server;
pthread_mutex_t mutex;
pthread_cond_t condvar;



void *client_handle(void* arg) {
    clientArg_t *client_arg = (clientArg_t *)arg;
    tcpsock_t *client = client_arg->client;
    sbuffer_t *send_buffer = client_arg->buffer;
    sensor_data_t data;
    data.flag = THREAD_INSERTED;
    int bytes, result, connected;
    connected = 0;
    int connected_id = -1;

    printf("start 1 thread, current start thread: %d\n",start_thread);
    do {
        //sbuffer_init(&send_buffer);
        // read sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *) &data.id, &bytes);
        // read temperature
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *) &data.value, &bytes);
        // read timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *) &data.ts, &bytes);

        if ((result == TCP_NO_ERROR) && bytes) {
            printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
                   (long int) data.ts);
            sbuffer_insert(send_buffer,&data);
        }
        if (connected ==0) {
            write_to_log_process("Sensor node %d has opened a new connection",data.id);
            connected_id = data.id;
            connected =1;
        }
        printf("TCP status: %d\n", result);
    } while (result == TCP_NO_ERROR);
    if (write_to_log_process("Sensor node %d has closed the connection",connected_id)==0) {
        tcp_close(&client);
        printf("client shut down\n");
        return 0;
    }
    return 0;
}




int start_server(void* passArg) {
    printf("starting thread: connmgr\n");
    serverArg_t* serverArg = (serverArg_t*)passArg;
    MAX_CONN = serverArg->MAX_CONN;
    PORT = serverArg->PORT;
    sbuffer_t *buffer = serverArg->buffer;

    pthread_mutex_init(&mutex, NULL);
    //pthread_cond_init(&condvar, NULL);

    pthread_t client_tid[MAX_CONN];
    tcpsock_t *client;

    int bytes, result;
    printf("start listening on the port: %d, the max connection is: %d\n", PORT, MAX_CONN);
    printf("Test server is started\n");
    // TCP server
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);

    while (1) {
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) {
            perror("Error accepting client connection");
            continue; // Skip to the next iteration
        }
        if (start_thread <= MAX_CONN){
            clientArg_t *client_arg = (clientArg_t *)malloc(sizeof(clientArg_t));
            client_arg->client  = client;
            client_arg->buffer = buffer;
            if (pthread_create(&client_tid[start_thread], NULL, client_handle, client_arg) != 0) {
                perror("Error creating thread");
                tcp_close(&client);
            }
        start_thread++;
        }
        if (start_thread == MAX_CONN){
            break;
        }

    }
    for (int i=0; i<MAX_CONN; i++) { // Wait for every thread to end
        pthread_join(client_tid[i], NULL);
    }
    sensor_data_t end_data;
    end_data.flag = THREAD_END;
    end_data.id = 1;
    end_data.value = 0;
    end_data.ts = 0;
    sbuffer_insert(buffer,&end_data);
    //pthread_cond_signal(&condvar);
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);

    printf("Test server is shutting down\n");

    return 0;
}