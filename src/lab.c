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
    // Ensure capacity is positive value
    if (capacity <= 0) {
        return NULL;
    }
    // Allocate memory
    queue_t q = malloc(sizeof(struct queue));
    if (q == NULL) { // Check for allocation failure
        return NULL;  
    }
    // Allocate memory for buffer.
    q->buffer = malloc(sizeof(void *) * capacity);
    if (q->buffer == NULL) {
        free(q);
        return NULL;
    }
    // Set values
    q->capacity = capacity;
    q->count = 0;
    q->head = 0;
    q->tail = 0;
    q->shutdown = false;
    // Handle mutex for thread safety, create condition variables, then return. 
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_full, NULL); // producers wait if queue is full
    pthread_cond_init(&q->not_empty, NULL); // consumers wait if queue is empty
    return q;
}

/**
 * @brief Frees all resources associated with the queue.
 *        Should signal all waiting threads so they can exit properly.
 *
 * @param q The queue to destroy.
 */
void queue_destroy(queue_t q) {
    // Nothing to destroy if queue is NULL
    if (q == NULL) {
        return;
    }
    // Lock the mutex to safely update shared data.
    pthread_mutex_lock(&q->lock);
    // Set shutdown flag to true so waiting threads can know to exit.
    q->shutdown = true;
    // Wake up all threads waiting on not_full (producers) and not_empty (consumers).
    pthread_cond_broadcast(&q->not_full);
    pthread_cond_broadcast(&q->not_empty);
    // Unlock the mutex after setting shutdown and signaling condition variables.
    pthread_mutex_unlock(&q->lock);
    // Destroy the mutex and condition variables now that no threads should be waiting.
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->not_full);
    pthread_cond_destroy(&q->not_empty);
    // Free the circular buffer and the queue structure itself.
    free(q->buffer);
    free(q);
}

/**
 * @brief Adds an element to the back of the queue.
 *        If the queue is full, this call should block until space is available.
 *
 * @param q The queue.
 * @param data The data to add.
 */
void enqueue(queue_t q, void *data) {
    // Do nothing if the queue or data is invalid.
    if (q == NULL || data == NULL) {
        return;
    }
    // Lock the mutex to safely access shared data.
    pthread_mutex_lock(&q->lock);
    // Wait while the queue is full and shutdown has NOT been called.
    while ( (q->count == q->capacity) && !q->shutdown ) {
        pthread_cond_wait(&q->not_full, &q->lock); // release the mutex while waiting, re-locks it after signaled.
    }
    // If shutdown was called while waiting, exit early.
    if (q->shutdown) {
        pthread_mutex_unlock(&q->lock);
        return;
    }
    // Add the data to the tail of the buffer.
    q->buffer[q->tail] = data;
    q->tail = (q->tail+1) % q->capacity; // Wrap around (circular buffer).
    q->count++; // Increase the count of items in the queue.
    // Signal one waiting consumer (if any) that the queue is no longer empty.
    pthread_cond_signal(&q->not_empty);
    // Unlock the mutex when done modifying the queue.
    pthread_mutex_unlock(&q->lock);
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
    // Ensure queue validity
    if (q == NULL) {
        return NULL;
    }
    // Lock the mutex to safely access shared data.
    pthread_mutex_lock(&q->lock);
    // Wait while the queue is empty and shutdown has NOT been called.
    while ( (q->count == 0) && !q->shutdown ) {
        pthread_cond_wait(&q->not_empty, &q->lock); // release the mutex while waiting, re-locks it after signaled.
    }
    // If shutdown was called and the queue is empty, exit and return NULL.
    if ( q->shutdown && (q->count == 0) ) {
        pthread_mutex_unlock(&q->lock);
        return NULL;
    }
    // Remove the item from the head of the buffer.
    void *data = q->buffer[q->head];
    q->head = (q->head+1) % q->capacity; // Wrap around (circular buffer).
    q->count--; // Decrease the count of items in the queue.
    // Signal one waiting producer (if any) that the queue is no longer full.
    pthread_cond_signal(&q->not_full);
    // Unlock the mutex when done modifying the queue.
    pthread_mutex_unlock(&q->lock);
    return data; // Return the dequeued item.
}

/**
 * @brief Sets the shutdown flag on the queue and signals all waiting threads.
 *
 * @param q The queue.
 */
void queue_shutdown(queue_t q) {
    // Ensure queue validity
    if (q == NULL) {
        return;
    }
    // Lock the mutex to safely update shared data.
    pthread_mutex_lock(&q->lock);
    // Set shutdown flag to true.
    q->shutdown = true;
    // Wake up all producers waiting on not_full and all consumers waiting on not_empty.
    pthread_cond_broadcast(&q->not_full);
    pthread_cond_broadcast(&q->not_empty);
    // Unlock the mutex after updating the shutdown flag and signaling threads.
    pthread_mutex_unlock(&q->lock);
}

/**
 * @brief Returns true if the queue is empty, false otherwise.
 *
 * @param q The queue.
 * @return True if the queue is empty, false otherwise.
 */
bool is_empty(queue_t q) {
    if (q == NULL) {
        return true;
    }
    // Lock the mutex to safely read shared data.
    pthread_mutex_lock(&q->lock);
    // Check if the number of items in the queue is zero.
    bool result = (q->count == 0);
    // Unlock the mutex after reading the shared data.
    pthread_mutex_unlock(&q->lock);
    return result; // Return whether the queue is empty.
}

/**
 * @brief Returns true if shutdown has been called on the queue.
 *
 * @param q The queue.
 * @return True if the queue is shutdown, false otherwise.
 */
bool is_shutdown(queue_t q) {
    if (q == NULL) {
        return false;
    }
    // Lock the mutex to safely read the shutdown flag.
    pthread_mutex_lock(&q->lock);
    // Check the shutdown flag.
    bool result = q->shutdown;
    // Unlock the mutex after reading the flag.
    pthread_mutex_unlock(&q->lock);
    return result; // Return whether shutdown has been set.
}
