/**
 * @file   aq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Alarm queue skeleton implementation
 */
#include "stdlib.h"
#include "aq.h"
// Define the internal queue structure
typedef struct MessageNode {
    void *msg;
    struct MessageNode *next;
} MessageNode;

typedef struct {
    MessageNode *normal_head; // Head of normal message queue
    MessageNode *normal_tail; // Tail of normal message queue
    void *alarm_msg;          // Pointer for the single alarm message (if present)
    int msg_count;            // Total message count
} QueueStruct;

AlarmQueue aq_create( ) {
    QueueStruct *queue = malloc(sizeof(QueueStruct));
    if (!queue) return NULL; // Return NULL if memory allocation fails
    queue->normal_head = NULL;
    queue->normal_tail = NULL;
    queue->alarm_msg = NULL;
    queue->msg_count = 0;
    return (AlarmQueue)queue;
}

int aq_send( AlarmQueue aq, void * msg, MsgKind k){
    QueueStruct *queue = (QueueStruct *)aq;

    if (k == AQ_ALARM) {
        // If an alarm message already exists, return AQNOROOM
        if (queue->alarm_msg != NULL) return AQ_NO_ROOM;
        // Set the alarm message if slot is empty
        queue->alarm_msg = msg;
    } else {
        // Insert normal messages at the end of the queue
        MessageNode *new_node = malloc(sizeof(MessageNode));
        if (!new_node) return -1; // Return error if memory allocation fails
        new_node->msg = msg;
        new_node->next = NULL;
        if (queue->normal_tail) {
            queue->normal_tail->next = new_node;
        } else {
            queue->normal_head = new_node; // Initialize head if queue was empty
        }
        queue->normal_tail = new_node;
    }

    queue->msg_count++;
    return 0; // Success
}

int aq_recv( AlarmQueue aq, void * * msg) {
    QueueStruct *queue = (QueueStruct *)aq;

    if (queue->alarm_msg) {
        // Return alarm message if present
        *msg = queue->alarm_msg;
        queue->alarm_msg = NULL; // Clear alarm message slot
        queue->msg_count--;
        return AQ_ALARM;
    } else if (queue->normal_head) {
        // Return normal message in FIFO order
        MessageNode *node = queue->normal_head;
        *msg = node->msg;
        queue->normal_head = node->next;
        if (!queue->normal_head) queue->normal_tail = NULL; // Queue empty
        free(node);
        queue->msg_count--;
        return AQ_NORMAL;
    }

    return AQ_NO_MSG; // No message available
}

int aq_size( AlarmQueue aq) {
    QueueStruct *queue = (QueueStruct *)aq;
    return queue->msg_count;
}

int aq_alarms( AlarmQueue aq) {
    QueueStruct *queue = (QueueStruct *)aq;
    return (queue->alarm_msg != NULL) ? 1 : 0;
}



