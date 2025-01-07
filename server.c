#include "server_handling.h"

queue_t *reqs;
Hashmap *map;
int num_threads = DEFAULT_THREADS;

void *worker_thread(void *arg) {
  (void)arg;

  while (1) {
    uintptr_t connfd = -1;
    q_pop(reqs, (void *)(int *)&connfd);
    recv_requests(connfd, map);
    close(connfd);
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    warnx("wrong arguments: %s port_num", argv[0]);
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    return EXIT_FAILURE;
  }

  // parse the command-line arguments
  int opt = 0;
  while ((opt = getopt(argc, argv, "t:")) != -1) {
    switch (opt) {
    case 't':
      num_threads = atoi(optarg);
      if (num_threads <= 0) {
        fprintf(stderr, "Invalid number of threads\n");
        return EXIT_FAILURE;
      }
      break;
    default:
      break;
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "Usage: %s [-t threads] <port>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char *endptr = NULL;
  size_t port = (size_t)strtoull(argv[optind], &endptr, 10);
  if (endptr && *endptr != '\0') {
    fprintf(stderr, "Invalid Port\n");
    return EXIT_FAILURE;
  }

  if (port < 1 || port > 65535) {
    fprintf(stderr, "Invalid Port\n");
    return EXIT_FAILURE;
  }

  signal(SIGPIPE, SIG_IGN);
  ConnectionListener sock;
  if (listener_init(&sock, port) < 0) {
    fprintf(stderr, "Invalid Port\n");
    return EXIT_FAILURE;
  }

  reqs = q_create(num_threads);
  map = hashmap_create(num_threads * 2);

  pthread_t threads[num_threads];
  for (int i = 0; i < num_threads; i++) {
    pthread_create(&(threads[i]), NULL, worker_thread, NULL);
  }

  // dispatcher thread
  while (1) {
    uintptr_t connfd = listener_accept(&sock);
    q_push(reqs, (void *)connfd);
  }

  for (int i = 0; i < num_threads; i++) {
    pthread_cancel(threads[i]);
    pthread_join(threads[i], NULL);
  }

  q_destroy(&reqs);
  hashmap_destroy(&map);

  return EXIT_SUCCESS;
}
