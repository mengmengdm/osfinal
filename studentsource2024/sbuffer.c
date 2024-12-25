//
// Created by fhr on 12/22/24.
//
/**
 * \author {AUTHOR}
 */

#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"
#include <pthread.h>



pthread_mutex_t mutex_sbuffer;
pthread_cond_t condvar_sbuffer;



int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    pthread_mutex_init(&mutex_sbuffer, NULL);
    pthread_cond_init(&condvar_sbuffer, NULL);
    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    sbuffer_node_t *dummy;
    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }
    //start, lock
    pthread_mutex_lock(&mutex_sbuffer);
    while ((*buffer)->head) {
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }
    free(*buffer);
    *buffer = NULL;
    pthread_mutex_unlock(&mutex_sbuffer); // unlock

    pthread_mutex_destroy(&mutex_sbuffer);
    pthread_cond_destroy(&condvar_sbuffer);
    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    printf("starting removing, data id: %d\n", data->flag);

    sbuffer_node_t *dummy;

    if (data->flag == 1) return SBUFFER_SUCCESS;
    if (buffer == NULL && data->flag == 0) return SBUFFER_FAILURE;

    //start operating on buffer, lock
    pthread_mutex_lock(&mutex_sbuffer);

    if (buffer->head == NULL) {
        pthread_cond_wait(&condvar_sbuffer, &mutex_sbuffer);
    }

    *data = buffer->head->data;
    dummy = buffer->head;
    if (buffer->head == buffer->tail) // buffer has only one node
    {
        buffer->head = buffer->tail = NULL;
    } else  // buffer has many nodes empty
    {
        buffer->head = buffer->head->next;
    }
    free(dummy);
    //finish, unlock
    pthread_mutex_unlock(&mutex_sbuffer);
    return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    //init the insert node
    sbuffer_node_t *dummy;
    if (buffer == NULL) {
        printf("failed to fetch the buffer while inserting\n");
        return SBUFFER_FAILURE;
    }
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) {
        printf("failed to initialize dummy while inserting\n");
        return SBUFFER_FAILURE;
    }

    dummy->data = *data;
    dummy->next = NULL;

    //strat to insert, lock buffer
    pthread_mutex_lock(&mutex_sbuffer);
    if (buffer->tail == NULL) // buffer empty (buffer->head should also be NULL
    {
        buffer->head = buffer->tail = dummy;
    } else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }

    //ending operation, unlock
    pthread_mutex_unlock(&mutex_sbuffer);
    printf("buffer inserted\n");
    //signal for reading
    pthread_cond_signal(&condvar_sbuffer);
    return SBUFFER_SUCCESS;
}
