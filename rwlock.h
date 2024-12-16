/**
 * @File rwlock.h
 *
 * A header file for a read/write lock to allow multithreading
 *
 * @author Vince Miguel S. Moreno
 */

#pragma once
#include <stdint.h>

typedef struct rwlock rwlock_t;

typedef enum { READERS, WRITERS, N_WAY } PRIORITY;

/** @brief Dynamically allocates and initializes a new rwlock with
 *         priority p, and, if using N_WAY priority, n.
 *
 *  @param The priority of the rwlock
 *
 *  @param The n value, if using N_WAY priority
 *
 *  @return a pointer to a new rwlock_t
 */
rwlock_t *rwlock_new(PRIORITY p, uint32_t n);

/** @brief Delete your rwlock and free all of its memory.
 *
 *  @param rw the rwlock to be deleted
 */
void rwlock_delete(rwlock_t **rw);

/** @brief acquire rw for reading
 *
 *  @param rw The lock to be acquired
 */
void reader_lock(rwlock_t *rw);

/** @brief release rw for reading
 *
 *  @param rw The lock to be released
 */
void reader_unlock(rwlock_t *rw);

/** @brief acquire rw for writing
 *
 *  @param rw The lock to be acquired
 */
void writer_lock(rwlock_t *rw);

/** @brief release rw for writing
 *
 *  @param rw The lock to be released
 */
void writer_unlock(rwlock_t *rw);
