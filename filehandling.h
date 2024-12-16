/**
 *  @File filehandling.h
 *
 *  Header file for file handling
 *
 *  @author Vince Miguel S. Moreno
 */

#pragma once
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>

/** @brief Reads bytes from fd into buf until either
 *  1. It has read n bytes
 *  2. There no bytes left to read in fd
 *  3. fd timeout or error
 *  4. The string is found
 *
 * @param fd The file descriptor or socket from which to read
 *
 * @param buf The buffer to put read data
 *
 * @param n The max amount of bytes to read. Constraint: n <= sizeof(buf)
 *
 * @param str The string to search for. NULL if there is no string
 *
 * @return The number of bytes read or -1 for error
 */
ssize_t read_until(int fd, char buf[], size_t n, char *str);

/** @brief Reads bytes from fd into buf until either
 * 1. It has read n bytes
 * 2. There are no bytes left to read in fd
 * 3. There is an error reading bytes
 *
 * @param fd The file descriptor or socket from which to read
 *
 * @param buf The buffer to put read data
 *
 * @param n The max amount of butes to read. Constraint: n <= sizeof(buf)
 *
 * @return The number of bytes read or -1 for error
 */
ssize_t nread(int fd, char buf[], size_t n);

/** @brief Writes bytes to fd from buf until either
 * 1. It writes exactly n bytes
 * 2. encounters a write error
 *
 * @param fd The file descriptor or socket to write to
 *
 * @param buf The buffer containing the data to write
 *
 * @param n The number of bytes to write. Constraint: n <= sizeof(buf)
 *
 * @return The number of bytes written or -1 for error
 */
ssize_t nwrite(int fd, char buf[], size_t n);

/** @brief Reads bytes from src and places them in dst until either
 * 1. It has read/written exactly n bytes
 * 2. No bytes left to read from src
 * 3. Read/write error
 *
 * @param src The file descripter or socket to read from
 *
 * @param dst The file descriptor or socket to write to
 *
 * @param n The number of bytes to read/write
 *
 * @@return The number of bytes written, or -1 on error
 */
ssize_t filepass(int src, int dst, size_t n);
