#include "response.h"
#include <stdint.h>
#include <string.h>

/**
 * @struct Response
 * @brief Represents an HTTP response code and message pair
 */
struct Response {
  uint16_t code;
  const char *message;
};

const Response_t RESPONSE_OK = {200, "OK"};
const Response_t RESPONSE_CREATED = {201, "Created"};
const Response_t RESPONSE_BAD_REQUEST = {400, "Bad Request"};
const Response_t RESPONSE_FORBIDDEN = {403, "Forbidden"};
const Response_t REPSONSE_NOT_FOUND = {404, "Not Found"};
const Response_t RESPONSE_INTERNAL_SERVER_ERROR = {500,
                                                   "Internal Server Error"};
const Response_t RESPONSE_NOT_IMPLEMENTED = {501, "Not Implemented"};
const Response_t RESPONSE_VERSION_NOT_SUPPORTED = {
    505, "HTTP Version Not Supported"};

/**
 * @brief Gets the HTTP response code from the Response_t obj
 *
 * @param response The response obj
 * @return The HTTP response code
 */
uint16_t response_get_code(const Response_t *response) {
  if (response == NULL)
    return 0;
  return response->code;
}

/**
 * @brief Gets the HTTP response message from the Response_t obj
 *
 * @param response The response obj
 * @return The HTTP response message
 */
const char *response_get_message(const Response_t *response) {
  if (response == NULL)
    return NULL;
  return response->message;
}
