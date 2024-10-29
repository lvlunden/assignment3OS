/**
 * @file   aq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Alarm queue skeleton implementation
 */

#include "aq.h"
#include <stdlib.h>


/**
 * @author s235064
 * @param aq_create()
 *
 */


//Our node structure
typedef struct Node {
    void *msg;
    MsgKind kind;
    struct Node *next;
} Node;


//Our pointers to the heads and tails of the queues
typedef struct {
    Node *alarm_head;
    Node *alarm_tail;
    Node *normal_head;
    Node *normal_tail;
    int alarm_count;
    int normal_count;
} Queue;

//The alarm queue
AlarmQueue aq_create( ) {
    Queue *newQueue = (Queue *)malloc(sizeof(Queue));
    if(newQueue) {
        newQueue->alarm_head = NULL;
        newQueue->alarm_tail = NULL;
        newQueue->normal_head = NULL;
        newQueue->normal_tail = NULL;
        newQueue->alarm_count = 0;
        newQueue->normal_count = 0;
    }
    return (AlarmQueue)newQueue;
}

int aq_send(AlarmQueue aq, void * msg, MsgKind k) {
    //To handle if not an alarm message or normal message
    if (!aq) {
        return AQ_NOT_IMPL;
    }
    if (!msg) {
        return AQ_NULL_MSG;
    }
    Queue *queue = (Queue *)aq;

    //if it is alarm message
    if (k == AQ_ALARM && queue->alarm_count > 0) {
        return AQ_NO_ROOM;
    }

    //Allocate new node for our message
    Node *newNode = (Node *)malloc(sizeof(Node));
    if(!newNode) {
        return AQ_NO_ROOM;
    }

    //to
    newNode->msg = msg;
    newNode->kind = k;
    newNode->next = NULL;


    //To handle if it is an alarm message
    if (k == AQ_ALARM) {
        // Add to the alarm queue
        if (queue->alarm_tail) {
            queue->alarm_tail->next = newNode;
        } else {
            queue->alarm_head = newNode;
        }
        queue->alarm_tail = newNode;
        queue->alarm_count++;
    } else {
        // Add to the normal queue
        if (queue->normal_tail) {
            queue->normal_tail->next = newNode;
        } else {
            queue->normal_head = newNode;
        }
        queue->normal_tail = newNode;
        queue->normal_count++;
    }

    return 0;
}

//HAVENT IMPLEMENTED MORE FROM HERE ON



int aq_recv( AlarmQueue aq, void * * msg) {
    //If there is no alarm queue
    if (!aq) {
        return AQ_UNINIT; // Queue has not been initialized
    }

    Queue *queue = (Queue *)aq;
    Node *node = NULL;
    MsgKind kind;

    // Check the alarm queue first
    if (queue->alarm_count > 0) {
        node = queue->alarm_head;
        *msg = node->msg;
        kind = AQ_ALARM;

        // Update the alarm queue's head to the next node (Not sure if necessary as we can have only one alarm msg in the queue)
        queue->alarm_head = node->next;
        if (!queue->alarm_head) {
            queue->alarm_tail = NULL;
        }

        queue->alarm_count--;
    } else if (queue->normal_count > 0) {
        // If no alarms, check the normal queue
        node = queue->normal_head;
        *msg = node->msg;
        kind = AQ_NORMAL;

        // Update the normal queue's head to the next node
        queue->normal_head = node->next;
        if (!queue->normal_head) {
            queue->normal_tail = NULL;
        }

        queue->normal_count--;
    } else {
        return AQ_NO_MSG; // No messages in either queue
    }

    free(node);
    return kind; // Return the kind of the message received
}



int aq_size( AlarmQueue aq) {
    if (!aq) {
        return AQ_UNINIT; // Queue has not been initialized
    }
    Queue *queue = (Queue *)aq;
    return queue->alarm_count + queue->normal_count;
}


int aq_alarms( AlarmQueue aq) {
    if (!aq) {
        return AQ_UNINIT; // Queue has not been initialized
    }
    Queue *queue = (Queue *)aq;
    return queue->alarm_count;
}


void aq_destroy(AlarmQueue aq) {
    if (!aq) {
        return;
    }

    Queue *queue = (Queue *)aq;
    Node *current = queue->alarm_head;
    Node *next;

    // Free all nodes in the alarm queue
    while (current) {
        next = current->next;
        free(current);
        current = next;
    }

    // Free all nodes in the normal queue
    current = queue->normal_head;
    while (current) {
        next = current->next;
        free(current);
        current = next;
    }

    free(queue);
}


