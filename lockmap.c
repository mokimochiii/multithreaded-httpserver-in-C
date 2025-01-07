#include "lockmap.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct Node {
  char *key;
  rwlock_t *value;
  struct Node *next;
};

struct Hashmap {
  Node **buckets;
  int num_buckets;
  pthread_mutex_t maplock;
};

Hashmap *hashmap_create(int num_buckets) {
  Hashmap *map = (Hashmap *)malloc(sizeof(Hashmap));
  map->num_buckets = num_buckets;
  map->buckets = calloc(num_buckets, sizeof(Node *));
  pthread_mutex_init(&map->maplock, NULL);
  return map;
}

void hashmap_destroy(Hashmap **map) {
  for (int i = 0; i < (*map)->num_buckets; i++) {
    Node *curr = (*map)->buckets[i];
    while (curr != NULL) {
      Node *next = curr->next;
      free(curr->key);
      rwlock_delete(&curr->value);
      free(curr);
      curr = next;
    }
  }
  free((*map)->buckets);
  pthread_mutex_destroy(&(*map)->maplock);
  free(*map);
  map = NULL;
}

int hash_function(char *key, int num_buckets) {
  int hash = 0;
  int p = 31;
  int m = num_buckets;
  for (int i = 0; key[i] != '\0'; i++) {
    hash = (hash * p + key[i]) % m;
  }
  return hash;
}

rwlock_t *get_rwlock(struct Hashmap *map, char *key) {
  if (!map || !key)
    return NULL;
  int bucket = hash_function(key, map->num_buckets);
  pthread_mutex_lock(&map->maplock);

  Node *curr = map->buckets[bucket];
  while (curr != NULL) {
    if (strcmp(curr->key, key) == 0) {
      pthread_mutex_unlock(&map->maplock);
      return curr->value;
    }
    curr = curr->next;
  }

  struct rwlock *new_lock = rwlock_new(WRITERS, 10);
  Node *new_node = (Node *)malloc(sizeof(Node));
  new_node->key = strdup(key);
  new_node->value = new_lock;
  new_node->next = map->buckets[bucket];
  map->buckets[bucket] = new_node;

  pthread_mutex_unlock(&map->maplock);
  return new_lock;
}
