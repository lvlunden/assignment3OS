/**
 * @file   aq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Alarm queue skeleton implementation
 */

#include "aq.h"
#include <pthread.h>
#include <stdlib.h>

typedef struct MessageNode {
    void *msg;
    MsgKind kind;
    struct MessageNode *next;
} MessageNode;

typedef struct {
    MessageNode *head;
    MessageNode *tail;
    int size;
    int alarm_count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t alarm_space;
} AlarmQueueStruct;

AlarmQueue aq_create( ) {
    AlarmQueueStruct *aq = malloc(sizeof(AlarmQueueStruct));
    if (!aq) return NULL;

    aq->head = NULL;
    aq->tail = NULL;
    aq->size = 0;
    aq->alarm_count = 0;

    // Initialize mutex and condition variables
    pthread_mutex_init(&aq->mutex, NULL);
    pthread_cond_init(&aq->not_empty, NULL);
    pthread_cond_init(&aq->alarm_space, NULL);

    return (AlarmQueue) aq;
}

int aq_send( AlarmQueue aq, void * msg, MsgKind k){
    if (!msg) return AQ_NULL_MSG;

    AlarmQueueStruct *queue = (AlarmQueueStruct *)aq;

    pthread_mutex_lock(&queue->mutex);

    if (k == AQ_ALARM && queue->alarm_count > 0) {
        // Queue already has an alarm message
        pthread_mutex_unlock(&queue->mutex);
        return AQ_NO_ROOM;
    }

    MessageNode *node = malloc(sizeof(MessageNode));
    if (!node) {
        pthread_mutex_unlock(&queue->mutex);
        return AQ_NO_ROOM; // Or another error code for allocation failure
    }

    node->msg = msg;
    node->kind = k;
    node->next = NULL;

    // Insert the node at the end of the queue
    if (queue->tail) {
        queue->tail->next = node;
        queue->tail = node;
    } else {
        queue->head = queue->tail = node;
    }

    queue->size++;
    if (k == AQ_ALARM) {
        queue->alarm_count = 1;
    }

    pthread_cond_signal(&queue->not_empty); // Signal any waiting receivers
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

int aq_recv( AlarmQueue aq, void * * msg) {
    if (!msg) return AQ_NULL_MSG;

    AlarmQueueStruct *queue = (AlarmQueueStruct *)aq;
    pthread_mutex_lock(&queue->mutex);

    while (queue->size == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }

    // Priority to alarm messages
    MessageNode *node = queue->head;
    MessageNode *prev = NULL;
    while (node && node->kind != AQ_ALARM) {
        prev = node;
        node = node->next;
    }

    if (!node) {
        // If no alarm, dequeue the first normal message
        node = queue->head;
    }

    if (prev) {
        prev->next = node->next;
    } else {
        queue->head = node->next;
    }
    if (node == queue->tail) {
        queue->tail = prev;
    }

    *msg = node->msg;
    int msg_kind = node->kind;

    if (msg_kind == AQ_ALARM) {
        queue->alarm_count = 0;
        pthread_cond_signal(&queue->alarm_space); // Signal space for another alarm
    }

    free(node);
    queue->size--;

    pthread_mutex_unlock(&queue->mutex);
    return msg_kind;
}

int aq_size( AlarmQueue aq) {
    AlarmQueueStruct *queue = (AlarmQueueStruct *)aq;
    pthread_mutex_lock(&queue->mutex);
    int size = queue->size;
    pthread_mutex_unlock(&queue->mutex);
    return size;
}

int aq_alarms( AlarmQueue aq) {
    AlarmQueueStruct *queue = (AlarmQueueStruct *)aq;
    pthread_mutex_lock(&queue->mutex);
    int alarms = queue->alarm_count;
    pthread_mutex_unlock(&queue->mutex);
    return alarms;
}



