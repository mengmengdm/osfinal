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
pthread_cond_t condvar_insert2reading;
pthread_cond_t condvar_reading2remove;



int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    pthread_mutex_init(&mutex_sbuffer, NULL);
    pthread_cond_init(&condvar_insert2reading, NULL);
    pthread_cond_init(&condvar_reading2remove,NULL);
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
    pthread_cond_destroy(&condvar_reading2remove);
    pthread_cond_destroy(&condvar_insert2reading);
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

    if (dummy->data.flag != THREAD_END) {
        dummy->data.flag = THREAD_READYTOREAD;
    }

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
    pthread_cond_signal(&condvar_insert2reading);
    return SBUFFER_SUCCESS;
}

int sbuffer_read(sbuffer_t *buffer, sensor_data_t *data) {
    pthread_mutex_lock(&mutex_sbuffer);
    if (buffer == NULL||buffer->head ==NULL) {
        printf("SBUFFER: waiting for buffer\n");
        pthread_cond_wait(&condvar_insert2reading, &mutex_sbuffer);
    }
    if (buffer->head->data.id==1) {
        //printf("SBUFFER: find the end at read");
        pthread_mutex_unlock(&mutex_sbuffer);
        pthread_cond_signal(&condvar_reading2remove);
        return SBUFFER_SUCCESS;
    }

    else{
        //buffer->head->data.flag != THREAD_READYTOREAD
        printf("current buffer head flag %d\n",buffer->head->data.flag);
        printf("SBUFFER: waiting for insert\n");
        pthread_cond_wait(&condvar_insert2reading, &mutex_sbuffer);
    }

    //printf("Starting read operation\n");
    //if (buffer->head->data.flag != THREAD_READYTOREAD) {return SBUFFER_FAILURE;}


    // If the buffer is empty, wait for data to be added

    // Read the data from the head node without removing it
    *data = buffer->head->data;
    data->flag =THREAD_READYTOREMOVE;
    printf("SBUFFER: wake up by insert, start reading,\nreading content: %d,%f,%d\n",data->id,data->value,data->flag);
    pthread_mutex_unlock(&mutex_sbuffer);
    printf("SBUFFER: send signal for removing\n");
    pthread_cond_signal(&condvar_reading2remove);
    //pthread_cond_wait(&condvar_insert2reading, &mutex_sbuffer);
    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    printf("starting removing\n");

    if (buffer == NULL) return SBUFFER_FAILURE;
    //if (data->id == 1) return SBUFFER_SUCCESS;

    sbuffer_node_t *dummy;

    //start operating on buffer, lock
    pthread_mutex_lock(&mutex_sbuffer);

    if (buffer->head == NULL) {
        printf("SBUFFER: waiting for reading/inserting\n");
        pthread_cond_wait(&condvar_reading2remove, &mutex_sbuffer);
    }
    printf("SBUFFER: wake up by reading, start removing\n");
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
    //pthread_cond_signal(&condvar_reading2remove);
    return SBUFFER_SUCCESS;
}


