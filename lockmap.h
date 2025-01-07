/**
 *  @File lockmap.h
 *
 *  A header file for mapping specific files to
 *  their own personal rwlock
 *
 *  @author Vince Miguel S. Moreno
 */

#pragma once

#include "rwlock.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node Node;

typedef struct Hashmap Hashmap;

/** @brief Dynamically allocate an initialize a new hashmap
 *
 *  @param num_buckets The amount of unique slots to be initialized
 *
 *  @return a pointer to the new hashmap
 */
Hashmap *hashmap_create(int num_buckets);

/** @brief Delete a hashmap and free all allocated memory
 *
 * @param map The hashmap to be deleted
 */
void hashmap_destroy(Hashmap **map);

/** @brief A simple hashing function for mapping an inserted element
 *
 *  @param key the element to be inserted and hashed
 *
 *  @param num_buckets the size of the map
 *
 *  @return an integer specifying which bucket of the hashmap to map to
 */
int hash_function(char *key, int num_buckets);

/** @brief Get an rwlock for a file from a map or create one if it does not
 * exist
 *
 *  @param map The hashmap from which to search from or insert
 *
 *  @key The file name to search for in the hashmap to retrieve the rwlock
 *
 *  @return the rwlock associated with the file (key)
 */
rwlock_t *get_rwlock(struct Hashmap *map, char *key);
