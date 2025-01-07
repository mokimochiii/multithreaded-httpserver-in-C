#include "connection_handling.h"
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Connection *connection_new(int connfd) {
  Connection *conn = (Connection *)malloc(sizeof(Connection));
  if (conn == NULL) {
    fprintf(stderr, "Failed to allocate memory of Connection struct\n");
    return NULL;
  }

  conn->connfd = connfd;
  conn->uri = NULL;
  conn->request = NULL;
  conn->body_start = NULL;
  memset(conn->headers, 0, sizeof(conn->headers));
  return conn;
}

void connection_delete(Connection **conn) {
  if (conn && *conn) {
    if ((*conn)->uri)
      free((*conn)->uri);
    close((*conn)->connfd);
    free(*conn);
    *conn = NULL;
  }
}

const Response_t *connection_parse(Connection *conn) {
  if (!conn)
    return &RESPONSE_BAD_REQUEST;

  // read response into a buffer and turn into a string
  char buffer[4096];
  ssize_t bytesread = read(conn->connfd, buffer, sizeof(buffer));
  if (bytesread <= 0)
    return &RESPONSE_BAD_REQUEST;
  buffer[bytesread] = '\0';

  // parsing
  regex_t regex;
  const char *pattern = "^([A-Z]+) (/[^ ]*) HTTP/([0-9]\\.[0-9])\r\n";
  if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
    fprintf(stderr, "Regex compilation failed\n");
    return &RESPONSE_INTERNAL_SERVER_ERROR;
    ;
  }

  regmatch_t matches[4];
  if (regexec(&regex, buffer, 4, matches, 0) == 0) {
    // parsing request
    size_t method_len = matches[1].rm_eo - matches[1].rm_so;
    char method[method_len + 1];
    strncpy(method, buffer + matches[1].rm_so, method_len);
    method[method_len] = '\0';

    if (strcmp(method, "GET") == 0) {
      conn->request = &REQUEST_GET;
    } else if (strcmp(method, "PUT") == 0) {
      conn->request = &REQUEST_PUT;
    } else {
      conn->request = &REQUEST_UNSUPPORTED;
    }

    // parsing uri
    size_t uri_len = matches[2].rm_eo - matches[2].rm_so;
    conn->uri = (char *)malloc(uri_len + 1);
    if (conn->uri) {
      strncpy(conn->uri, buffer + matches[2].rm_so + 1, uri_len - 1);
      conn->uri[uri_len] = '\0';
    }

    size_t version_len = matches[3].rm_eo - matches[3].rm_so;
    char version[version_len + 1];
    strncpy(version, buffer + matches[3].rm_so, version_len);
    version[version_len] = '\0';

    if (strcmp(version, "1.1") != 0) {
      conn->request = &REQUEST_UNSUPPORTED;
      regfree(&regex);
      return &RESPONSE_VERSION_NOT_SUPPORTED;
    }

    char *header_start = strstr(buffer, "\r\n\r\n");

    if (header_start) {
      header_start += 4; // skip past \r\n\r\n
      size_t header_len = header_start - buffer;
      if (header_len < sizeof(conn->headers)) {
        strncpy(conn->headers, buffer, header_len);
        conn->headers[header_len] = '\0';
      }
      conn->body_start = header_start;
    }
  } else {
    conn->request = &REQUEST_UNSUPPORTED;
    regfree(&regex);
    return &RESPONSE_BAD_REQUEST;
  }

  regfree(&regex);
  return NULL;
}

const Request_t *connection_get_request(Connection *conn) {
  return conn ? conn->request : NULL;
}

char *connection_get_uri(Connection *conn) { return conn ? conn->uri : NULL; }

char *connection_get_header(Connection *conn, char *header) {
  if (!conn || !header)
    return NULL;

  if (strcmp(header, "Content-Length") == 0) {
    char *content_length = strstr(conn->headers, "Content-Length: ");
    if (!content_length)
      return NULL;
    content_length += strlen("Content-Length: ");
    char *end = strchr(content_length, '\r');
    if (!end)
      return NULL;
    size_t len = end - content_length;
    char *value = (char *)malloc(len + 1);
    if (!value)
      return NULL;
    strncpy(value, content_length, len);
    value[len] = '\0';
    return value;
  } else if (strcmp(header, "Request-Id") == 0) {
    char *request_id = strstr(conn->headers, "Request-Id: ");
    if (!request_id)
      return NULL;
    request_id += strlen("Request Id: ");
    char *end = strchr(request_id, '\r');
    if (!end)
      return NULL;
    size_t len = end - request_id;
    char *value = (char *)malloc(len + 1);
    if (!value)
      return NULL;
    strncpy(value, request_id, len);
    value[len] = '\0';
    return value;
  }

  return NULL;
}

const Response_t *connection_write_to_file(Connection *conn, int fd) {
  if (!conn || fd < 0)
    return &RESPONSE_BAD_REQUEST;

  char *content_length_str = connection_get_header(conn, "Content-Length");
  if (!content_length_str)
    return &RESPONSE_BAD_REQUEST;

  size_t content_length = atoi(content_length_str);
  free(content_length_str);

  if (write(fd, conn->body_start, content_length) != (ssize_t)content_length)
    return &RESPONSE_INTERNAL_SERVER_ERROR;

  return NULL;
}

const Response_t *connection_read_from_file(Connection *conn, int fd,
                                            uint64_t count) {
  if (!conn || fd < 0)
    return &RESPONSE_BAD_REQUEST;

  char buffer[4096];
  uint64_t remaining = count;

  while (remaining > 0) {
    size_t to_read = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
    ssize_t bytesread = nread(fd, buffer, to_read);

    if (bytesread <= 0)
      return &RESPONSE_INTERNAL_SERVER_ERROR;

    ssize_t byteswritten = nwrite(conn->connfd, buffer, bytesread);

    if (byteswritten < bytesread)
      return &RESPONSE_INTERNAL_SERVER_ERROR;

    remaining -= byteswritten;
  }

  return NULL;
}

const Response_t *connection_send_response(Connection *conn,
                                           const Response_t *response) {
  if (!conn || !response)
    return &RESPONSE_BAD_REQUEST;

  char buffer[256];
  int len =
      snprintf(buffer, sizeof(buffer), "HTTP/1.1 %d %s\r\n\r\n",
               response_get_code(response), response_get_message(response));
  if (write(conn->connfd, buffer, len) != len)
    return &RESPONSE_INTERNAL_SERVER_ERROR;

  return NULL;
}
