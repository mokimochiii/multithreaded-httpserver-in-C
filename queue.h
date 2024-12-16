/**
 * @File queue.h
 *
 * Header file for queue to use for queueing requests (multithreading)
 * The queue functions will also be implemented with critical regions
 * for multithreading
 *
 * @author Vince Miguel S. Moreno
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct queue queue_t;

/** @brief Dynamically allocates memory for and initializes a queue struct
 *
 *  @param size The max capacity of a queue
 *
 *  @return a pointer to the new queue
 */
queue_t *q_create(int size);

/** @brief Delete queue and free all allocated memory
 *
 * @param q The queue to be deleted
 *
 */
void q_destroy(queue_t **q);

/** @brief push an element onto a queue
 *
 *  @param q The queue to push onto
 *
 *  @param elem The element to add to the queue
 *
 *  @return A bool indicating true for success and false for failure
 */
bool q_push(queue_t *q, void *elem);

/** @brief pop an element from a queue
 *
 *  @param q The queue to pop from
 *
 *  @param elem A place to assign the popped element
 *
 *  @return A bool indicating true for success and false for failure
 */
bool q_pop(queue_t *q, void **elem);
