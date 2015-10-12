
#include "queue.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct QueueStruct {
    void **data;
    int front;
    int rear;
    int count;
    int size;

    // semaphore
    sem_t sem_enqueue;
    sem_t sem_dequeue;
    pthread_mutex_t mutex_lock;

} Queue;


Queue *queue_alloc(int size) {

    // allocate memory for queue struct
    Queue *queue = (Queue *) malloc (sizeof(Queue));

    // allocate memory for data
    queue->data = malloc(sizeof(void *) * size);

    // init queue value
    queue->front = 0;
    queue->rear = 0;
    queue->count = 0;
    queue->size = size;

    // init semaphore and mutex lock
    sem_init(&queue->sem_enqueue, 0, size);
    sem_init(&queue->sem_dequeue, 0, 0);
    pthread_mutex_init(&queue->mutex_lock, NULL);

    return queue;
}

void queue_free(Queue *queue) {
    int sval;
    sem_getvalue(&queue->sem_dequeue, &sval);

    if (sval != 0)
    {
        handle_error("Error: there are still items left in queue");
    }

    free(queue->data);
    free(queue);
}

void queue_put(Queue *queue, void *item) {

    // block
    sem_wait(&queue->sem_enqueue);

    // put item to the tail of queue
    pthread_mutex_lock(&queue->mutex_lock);

    queue->data[queue->rear++] = item;
    queue->rear %= queue->size;
    queue->count += 1;

    pthread_mutex_unlock(&queue->mutex_lock);
    sem_post(&queue->sem_dequeue);
}


void *queue_get(Queue *queue) {

    // block
    sem_wait(&queue->sem_dequeue);

    // fetch the item at front of queue
    pthread_mutex_lock(&queue->mutex_lock);
    void *item = queue->data[queue->front++];
    queue->front %= queue->size;
    queue->count -= 1;

    pthread_mutex_unlock(&queue->mutex_lock);
    sem_post(&queue->sem_enqueue);

    return item;

}
