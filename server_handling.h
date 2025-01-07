/**
 * @File server_handling.h
 *
 * A header file for abstracting httpserver processing and handling
 *
 * @author Vince Miguel S. Moreno
 */

#pragma once

#include "connection.h"
#include "connection_handling.h"
#include "filehandling.h"
#include "lockmap.h"
#include "queue.h"
#include "request.h"
#include "response.h"
#include "rwlock.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>
#include <pthread.h>
#include <sys/stat.h>

#define DEFAULT_THREADS 4
#define SIZE 300

void recv_requests(int, Hashmap *);
void handle_get(Connection *, Hashmap *);
void handle_put(Connection *, Hashmap *);
void handle_unsupported(Connection *);
