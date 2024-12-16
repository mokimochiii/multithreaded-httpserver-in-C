#include "request.h"
#include <stdint.h>
#include <stdlib.h>

/**
 * @struct Request
 * @brief Represents an HTTP request method
 */
struct Request {
  const char *method;
};

const Request_t REQUEST_GET = {"GET"};
const Request_t REQUEST_PUT = {"PUT"};
const Request_t REQUEST_UNSUPPORTED = {"UNSUPPORTED"};

const Request_t *requests[NUM_REQUESTS] = {&REQUEST_GET, &REQUEST_PUT,
                                           &REQUEST_UNSUPPORTED};

/**
 * @brief Gets the HTTP request method string from the Request_t obj
 *
 * @param request The request obj
 * @return The HTTP request method string
 */
const char *request_get_str(const Request_t *request) {
  if (request == NULL)
    return NULL;
  return request->method;
}
