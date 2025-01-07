#include "server_handling.h"

void recv_requests(int connfd, Hashmap *map) {
  ;
  Connection *conn = connection_new(connfd);
  const Response_t *res = connection_parse(conn);

  if (res != NULL) {
    connection_send_response(conn, res);
  } else {
    const Request_t *req = connection_get_request(conn);
    if (req == &REQUEST_GET) {
      handle_get(conn, map);
    } else if (req == &REQUEST_PUT) {
      handle_put(conn, map);
    } else {
      handle_unsupported(conn);
    }
  }

  connection_delete(&conn);
}

void handle_get(Connection *conn, Hashmap *map) {
  char *uri = connection_get_uri(conn);
  struct rwlock *lock = get_rwlock(map, uri);
  char *header = NULL;

  // start of critical region

  reader_lock(lock);

  // Open the file.
  int fd = open(uri, O_RDONLY);
  if (fd < 0) {
    reader_unlock(lock);
    if (errno == EACCES) {
      connection_send_response(conn, &RESPONSE_FORBIDDEN);
      // audit log
      if ((header = connection_get_header(conn, "Request-Id")) == NULL)
        header = "0";
      fprintf(stderr, "GET,/%s,403,%d\n", uri, atoi(header));
    } else if (errno == ENOENT) {
      connection_send_response(conn, &RESPONSE_NOT_FOUND);
      // audit log
      if ((header = connection_get_header(conn, "Request-Id")) == NULL)
        header = "0";
      fprintf(stderr, "GET,/%s,404,%d\n", uri, atoi(header));
    } else {
      connection_send_response(conn, &RESPONSE_INTERNAL_SERVER_ERROR);
      // audit log
      if ((header = connection_get_header(conn, "Request-Id")) == NULL)
        header = "0";
      fprintf(stderr, "GET,/%s,500,%d\n", uri, atoi(header));
    }
    return;
  }

  struct stat file_stat;
  if (fstat(fd, &file_stat) < 0) {
    reader_unlock(lock);
    connection_send_response(conn, &RESPONSE_INTERNAL_SERVER_ERROR);
    close(fd);
    return;
  }

  // Check if the file is a directory
  if (S_ISDIR(file_stat.st_mode)) {
    reader_unlock(lock);
    connection_send_response(conn, &RESPONSE_FORBIDDEN);
    // audit log
    if ((header = connection_get_header(conn, "Request-Id")) == NULL)
      header = "0";
    fprintf(stderr, "GET,/%s,403,%d\n", uri, atoi(header));
    close(fd);
    return;
  }

  // Send the file
  const Response_t *send_status =
      connection_read_from_file(conn, fd, file_stat.st_size);
  if (send_status != NULL) {
    connection_send_response(conn, send_status);
    return;
  }
  // audit log
  if ((header = connection_get_header(conn, "Request-Id")) == NULL)
    header = "0";
  fprintf(stderr, "GET,/%s,200,%d\n", uri, atoi(header));

  // Close the file
  reader_unlock(lock);
  close(fd);
}

void handle_put(Connection *conn, Hashmap *map) {
  char *uri = connection_get_uri(conn);
  struct rwlock *lock = get_rwlock(map, uri);

  // middleman

  FILE *temp = tmpfile();
  int temp_fd = fileno(temp);
  connection_write_to_file(conn, temp_fd);
  lseek(temp_fd, 0, SEEK_SET);
  struct stat t;
  fstat(temp_fd, &t);

  // start of critical region
  writer_lock(lock);

  // Check if file already exists before opening it
  char *header = NULL;
  bool isCreated = false;
  if (access(uri, F_OK) == 0) {
    if (access(uri, W_OK) < 0) {
      writer_unlock(lock);
      close(temp_fd);
      connection_send_response(conn, &RESPONSE_FORBIDDEN);
      if ((header = connection_get_header(conn, "Request-Id")) == NULL)
        header = "0";
      fprintf(stderr, "GET,/%s,403,%d\n", uri, atoi(header));
      return;
    }
    isCreated = true;
  }
  
  // Open the file.
  int fd = open(uri, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (fd < 0) {
    close(temp_fd);
    writer_unlock(lock);
    if (errno == EACCES) {
      connection_send_response(conn, &RESPONSE_FORBIDDEN);
      // audit log
      if ((header = connection_get_header(conn, "Request-Id")) == NULL)
        header = "0";
      fprintf(stderr, "GET,/%s,403,%d\n", uri, atoi(header));
    } else if (errno == EISDIR) {
      connection_send_response(conn, &RESPONSE_FORBIDDEN);
      // audit log
      if ((header = connection_get_header(conn, "Request-Id")) == NULL)
        header = "0";
      fprintf(stderr, "GET,/%s,403,%d\n", uri, atoi(header));
    } else {
      connection_send_response(conn, &RESPONSE_INTERNAL_SERVER_ERROR);
      // audit log
      if ((header = connection_get_header(conn, "Request-Id")) == NULL)
        header = "0";
      fprintf(stderr, "GET,/%s,500,%d\n", uri, atoi(header));
    }
    return;
  }

  // Receive the file
  filepass(temp_fd, fd, t.st_size);

  // Send the response
  if (isCreated) {
    connection_send_response(conn, &RESPONSE_OK);
    if ((header = connection_get_header(conn, "Request-Id")) == NULL)
      header = "0";
    fprintf(stderr, "PUT,/%s,200,%d\n", uri, atoi(header));
  } else {
    connection_send_response(conn, &RESPONSE_CREATED);
    if ((header = connection_get_header(conn, "Request-Id")) == NULL)
      header = "0";
    fprintf(stderr, "PUT,/%s,201,%d\n", uri, atoi(header));
  }

  // Close the file
  writer_unlock(lock);
  close(fd);
  close(temp_fd);
}

void handle_unsupported(Connection *conn) {
  connection_send_response(conn, &RESPONSE_NOT_IMPLEMENTED);
  // audit log
  char *uri = connection_get_uri(conn);
  char *header = NULL;
  if ((header = connection_get_header(conn, "Request-Id")) == NULL)
    header = "0";
  fprintf(stderr, "GET,/%s,501,%s\n", uri, header);
  fprintf(stderr, "UNSUPPORTED,/%s,501,%s\n", uri, header);
}
