/**
 * @file lab.c
 * @brief Bounded FIFO Queue Monitor Implementation
 * 
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lab.h"

/**
 * @brief Internal structure for the queue.
 *        Holds the buffer, capacity info, and synchronization primitives.
 */
struct queue {
    void **buffer;               // Array of void pointers (the circular buffer)
    int capacity;                // Maximum number of items in the queue
    int count;                   // Current number of items in the queue
    int head;                    // Index of the next item to dequeue
    int tail;                    // Index of the next slot to enqueue

    bool shutdown;               // Flag to indicate if shutdown has been called

    pthread_mutex_t lock;        // Mutex to protect shared data
    pthread_cond_t not_full;     // Condition variable for producer wait
    pthread_cond_t not_empty;    // Condition variable for consumer wait
};

/**
 * @brief Initializes a new queue with the given capacity.
 *
 * @param capacity The maximum number of items the queue can hold.
 * @return A pointer to the initialized queue.
 */
queue_t queue_init(int capacity) {
    // TODO
    return NULL;
}

/**
 * @brief Frees all resources associated with the queue.
 *        Should signal all waiting threads so they can exit properly.
 *
 * @param q The queue to destroy.
 */
void queue_destroy(queue_t q) {
    // TODO
}

/**
 * @brief Adds an element to the back of the queue.
 *        If the queue is full, this call should block until space is available.
 *
 * @param q The queue.
 * @param data The data to add.
 */
void enqueue(queue_t q, void *data) {
    // TODO
}

/**
 * @brief Removes and returns the first element in the queue.
 *        If the queue is empty, this call should block until an item is available.
 *        After shutdown, it may return NULL to allow consumers to exit.
 *
 * @param q The queue.
 * @return A pointer to the dequeued data, or NULL if shutdown.
 */
void *dequeue(queue_t q) {
    // TODO
    return NULL;
}

/**
 * @brief Sets the shutdown flag on the queue and signals all waiting threads.
 *
 * @param q The queue.
 */
void queue_shutdown(queue_t q) {
    // TODO
}

/**
 * @brief Returns true if the queue is empty, false otherwise.
 *
 * @param q The queue.
 * @return True if the queue is empty, false otherwise.
 */
bool is_empty(queue_t q) {
    // TODO
    return false;
}

/**
 * @brief Returns true if shutdown has been called on the queue.
 *
 * @param q The queue.
 * @return True if the queue is shutdown, false otherwise.
 */
bool is_shutdown(queue_t q) {
    // TODO
    return false;
}
