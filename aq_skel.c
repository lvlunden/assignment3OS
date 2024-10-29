/**
 * @file   aq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Alarm queue skeleton implementation
 */

#include "aq.h"
#include <stdlib.h>
#include <pthread.h>

typedef struct Node {
    void *msg;
    struct Node *next;
} Node;


typedef struct {
    Node *alarm_head;
    Node *alarm_tail;
    Node *normal_head;
    Node *normal_tail;
    int alarm_count;
    int normal_count;
    pthread_mutex_t lock;            // Mutex for queue access
    pthread_cond_t not_empty;        // Condition for non-empty queue
    pthread_cond_t alarm_slot_free;  // Condition for alarm slot availability
} Queue;




AlarmQueue aq_create( ) {
    Queue *newQueue = (Queue *)malloc(sizeof(Queue));
    if (!newQueue) {
       return NULL;
    }
        newQueue->alarm_head = NULL;
        newQueue->alarm_tail = NULL;
        newQueue->normal_head = NULL;
        newQueue->normal_tail = NULL;
        newQueue->alarm_count = 0;
        newQueue->normal_count = 0;
        pthread_mutex_init(&newQueue->lock, NULL);
        pthread_cond_init(&newQueue->not_empty, NULL);
        pthread_cond_init(&newQueue->alarm_slot_free, NULL);

    // Initialize mutex and condition variables, check for errors
    if (pthread_mutex_init(&newQueue->lock, NULL) != 0) {
        free(newQueue);
        return NULL;
    }
    if (pthread_cond_init(&newQueue->not_empty, NULL) != 0) {
        pthread_mutex_destroy(&newQueue->lock);
        free(newQueue);
        return NULL;
    }
    if (pthread_cond_init(&newQueue->alarm_slot_free, NULL) != 0) {
        pthread_cond_destroy(&newQueue->not_empty);
        pthread_mutex_destroy(&newQueue->lock);
        free(newQueue);
        return NULL;
    }
    return (AlarmQueue)newQueue;
}

int aq_send( AlarmQueue aq, void * msg, MsgKind k){
    if (!aq || !msg) return AQ_NULL_MSG;

    Queue *queue = (Queue *)aq;
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode) return AQ_NO_ROOM;

    newNode->msg = msg;
    newNode->next = NULL;

    pthread_mutex_lock(&queue->lock);

    if (k == AQ_ALARM) {
        while (queue->alarm_count > 0) {
            pthread_cond_wait(&queue->alarm_slot_free, &queue->lock);
        }
        queue->alarm_tail = queue->alarm_head = newNode;
        queue->alarm_count++;
    } else {
        if (queue->normal_tail) {
            queue->normal_tail->next = newNode;
        } else {
            queue->normal_head = newNode;
        }
        queue->normal_tail = newNode;
        queue->normal_count++;
    }

    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->lock);
    return 0;
}

int aq_recv( AlarmQueue aq, void * * msg) {
    if (!aq || !msg) return AQ_UNINIT;

    Queue *queue = (Queue *)aq;
    pthread_mutex_lock(&queue->lock);

    while (queue->alarm_count == 0 && queue->normal_count == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->lock);
    }

    Node *node = NULL;
    MsgKind kind;

    if (queue->alarm_count > 0) {
        node = queue->alarm_head;
        *msg = node->msg;
        kind = AQ_ALARM;
        queue->alarm_head = queue->alarm_tail = NULL;
        queue->alarm_count--;
        pthread_cond_signal(&queue->alarm_slot_free);
    } else if (queue->normal_count > 0) {
        node = queue->normal_head;
        *msg = node->msg;
        kind = AQ_NORMAL;
        queue->normal_head = queue->normal_head->next;
        if (!queue->normal_head) {
            queue->normal_tail = NULL;
        }
        queue->normal_count--;
    }

    free(node);
    pthread_mutex_unlock(&queue->lock);
    return kind;
}



//To get the total size of the queue
int aq_size( AlarmQueue aq) {
    if(!aq) return AQ_UNINIT;
    Queue *queue = (Queue *)aq;

    pthread_mutex_lock(&queue->lock);
    int size = queue->alarm_count + queue->normal_count;
    pthread_mutex_unlock(&queue->lock);
    return size;
}


//To get the count of alarm messages in its queue
int aq_alarms( AlarmQueue aq) {
    if (!aq) return AQ_UNINIT;
    Queue *queue = (Queue *)aq;
    pthread_mutex_lock(&queue->lock);
    int alarms = queue->alarm_count;
    pthread_mutex_unlock(&queue->lock);
  return alarms;
}


