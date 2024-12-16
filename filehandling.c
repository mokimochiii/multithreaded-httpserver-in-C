#include "filehandling.h"

ssize_t read_until(int fd, char buf[], size_t n, char *str) {
  ssize_t totalread = 0;
  ssize_t bytesread;

  while (totalread < (ssize_t)n) {
    bytesread = read(fd, buf + totalread, n - totalread);

    if (bytesread > 0) {
      totalread += bytesread;

      // temporarily set null terminator to check segment as a string
      buf[totalread] = '\0';

      if (str != NULL) {
        char *found = strstr(buf, str);
        if (found != NULL) {
          totalread = found - buf + strlen(str);
          break;
        }
      }
    } else if (bytesread == 0) {
      break;
    } else {
      if (errno == EINTR) {
        continue;
      } else {
        return -1;
      }
    }
  }

  return totalread;
}

ssize_t nread(int fd, char buf[], size_t n) {
  ssize_t totalread = 0;
  ssize_t bytesread;

  while (totalread < (ssize_t)n) {
    bytesread = read(fd, buf + totalread, n - totalread);

    if (bytesread > 0) {
      totalread += bytesread;
    } else if (bytesread == 0) {
      break;
    } else {
      if (errno == EINTR) {
        continue;
      } else {
        return -1;
      }
    }
  }
  return totalread;
}

ssize_t nwrite(int fd, char buf[], size_t n) {
  ssize_t totalwritten = 0;
  ssize_t byteswritten;

  while (totalwritten < (ssize_t)n) {
    byteswritten = write(fd, buf + totalwritten, n - totalwritten);
    if (byteswritten > 0) {
      totalwritten += byteswritten;
    } else if (byteswritten == 0) {
      break;
    } else {
      if (errno == EINTR) {
        continue;
      } else {
        return -1;
      }
    }
  }
  return totalwritten;
}

ssize_t filepass(int src, int dst, size_t n) {
  ssize_t totalwritten = 0;
  ssize_t bytesread;
  ssize_t byteswritten;
  char buf[1024];

  while (totalwritten < (ssize_t)n) {
    // set bytes to read either to buf size or whatevers left of the fd is
    // smaller than buf
    size_t bytes_to_read =
        (n - totalwritten < sizeof(buf)) ? (n - totalwritten) : sizeof(buf);
    bytesread = read(src, buf, bytes_to_read);

    if (bytesread > 0) {
      ssize_t written = 0;

      while (written < bytesread) {
        byteswritten = write(dst, buf + written, bytesread - written);

        if (byteswritten > 0) {
          written += byteswritten;
        } else if (byteswritten == -1 && errno == EINTR) {
          continue;
        } else {
          return -1;
        }
      }
      totalwritten += bytesread;
    } else if (bytesread == 0) {
      break;
    } else {
      if (errno == EINTR) {
        continue;
      } else {
        return -1;
      }
    }
  }
  return totalwritten;
}
