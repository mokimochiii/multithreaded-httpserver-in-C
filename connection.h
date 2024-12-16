/**
 * @File connection.h
 *
 * Header file for facilitating socket connection
 *
 * @author Vince Miguel S. Moreno
 */

#pragma once

#include <stdint.h>
#include <sys/types.h>
/** @struct ConnectionListener
 * @brief Represents a socket listening for connections
 */
typedef struct {
  int fd;
} ConnectionListener;

/** @brief Initializes a ConnectionListener that listens on the provided port
 *
 * @param sock The ConnectionListener to initialize
 *
 * @param port The port to listen to
 *
 * @return 0 on success or -1 on failure to listen
 */
int listener_init(ConnectionListener *sock, int port);

/** @brief Accept a new connection and initialize a 5 second timeout
 *
 * @param sock The ConnectionListener to get the connection from
 *
 * @return A socket for the connection or -1 on error
 */
int listener_accept(ConnectionListener *sock);
