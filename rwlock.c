#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "rwlock.h"

struct rwlock {
  PRIORITY priority;  // prio of rwlock
  int num;            // num readers in n-way mode
  pthread_mutex_t mu; // rwlock mutex
  pthread_cond_t rcv; // reader conditional var
  pthread_cond_t wcv; // writer conditional var
  int areaders;       // number of active readers
  int awriters;       // number of active writers
  int wreaders;       // number of waiting readers
  int wwriters;       // number of waiting writers
  int nwayreaders;    // number of readers in nway mode
};

rwlock_t *rwlock_new(PRIORITY p, uint32_t n) {
  rwlock_t *rwlock = (rwlock_t *)malloc(sizeof(rwlock_t));
  if (rwlock == NULL)
    return NULL;

  rwlock->priority = p;
  rwlock->num = n;

  pthread_mutex_init(&rwlock->mu, NULL);
  pthread_cond_init(&rwlock->rcv, NULL);
  pthread_cond_init(&rwlock->wcv, NULL);

  rwlock->areaders = 0;
  rwlock->awriters = 0;
  rwlock->wreaders = 0;
  rwlock->wwriters = 0;
  rwlock->nwayreaders = 0;
  return rwlock;
}

void rwlock_delete(rwlock_t **rwlock) {
  if (!rwlock || !*rwlock)
    return;

  pthread_mutex_destroy(&(*rwlock)->mu);
  pthread_cond_destroy(&(*rwlock)->rcv);
  pthread_cond_destroy(&(*rwlock)->wcv);

  free(*rwlock);
  *rwlock = NULL;
}

void reader_lock(rwlock_t *rwlock) {
  pthread_mutex_lock(&rwlock->mu); // start of critical region

  rwlock->wreaders++;

  while (rwlock->awriters > 0 ||
         (rwlock->priority == N_WAY && rwlock->nwayreaders >= rwlock->num &&
          rwlock->wreaders > 0) ||
         (rwlock->priority == WRITERS && rwlock->wwriters > 0)) {
    // conditions to block readers:
    // if there are active writers
    // if nway priority and there are more readers than allowed and there are
    // waiting readers if writers have priority and there are waiting writers
    pthread_cond_wait(&rwlock->rcv, &rwlock->mu);
  }

  rwlock->wreaders--;
  rwlock->areaders++;

  if (rwlock->priority == N_WAY)
    rwlock->nwayreaders++;

  pthread_mutex_unlock(&rwlock->mu); // end of critical region
}

void reader_unlock(rwlock_t *rwlock) {
  pthread_mutex_lock(&rwlock->mu);

  rwlock->areaders--;

  if (rwlock->areaders == 0 && rwlock->wwriters > 0) {
    pthread_cond_signal(&rwlock->wcv);
  } else {
    pthread_cond_broadcast(&rwlock->rcv);
  }

  pthread_mutex_unlock(&rwlock->mu);
}

void writer_lock(rwlock_t *rwlock) {
  pthread_mutex_lock(&rwlock->mu);

  rwlock->wwriters++;

  while (rwlock->areaders > 0 || rwlock->awriters > 0) {
    // conditions to block writers:
    // if there are active readers
    // if there are active writers
    pthread_cond_wait(&rwlock->wcv, &rwlock->mu);
  }

  rwlock->nwayreaders = 0;
  rwlock->wwriters--;
  rwlock->awriters++;

  pthread_mutex_unlock(&rwlock->mu);
}

void writer_unlock(rwlock_t *rwlock) {
  pthread_mutex_lock(&rwlock->mu);

  rwlock->awriters--;

  switch (rwlock->priority) {
  case WRITERS:
    if (rwlock->wwriters > 0) {
      // continue writing if there are waiting writers
      pthread_cond_broadcast(&rwlock->wcv);
    } else {
      pthread_cond_broadcast(&rwlock->rcv);
    }
  case READERS:
    if (rwlock->wreaders > 0) {
      pthread_cond_broadcast(&rwlock->rcv);
    } else if (rwlock->wwriters > 0) {
      pthread_cond_signal(&rwlock->wcv);
    }
  case N_WAY:
    if (rwlock->wreaders > 0) {
      pthread_cond_broadcast(&rwlock->rcv);
    } else if (rwlock->wwriters > 0) {
      pthread_cond_signal(&rwlock->wcv);
    }
  default:
    pthread_cond_signal(&rwlock->wcv);
  }

  pthread_mutex_unlock(&rwlock->mu);
}
