#pragma once

#include "filehandling.h"
#include "request.h"
#include "response.h"

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct Connection {
  int connfd;
  char *uri;
  const Request_t *request;
  char headers[1024];
  char *body_start;     
} Connection;

Connection *connection_new(int connfd);

void connection_delete(Connection **conn);

const Response_t *connection_parse(Connection *conn);

const Request_t *connection_get_request(Connection *conn);

char *connection_get_uri(Connection *conn);

char *connection_get_header(Connection *conn, char *header);

const Response_t *connection_write_to_file(Connection *conn, int fd);

const Response_t *connection_read_from_file(Connection *conn, int fd,
                                            uint64_t count);

const Response_t *connection_send_response(Connection *conn,
                                           const Response_t *response);
