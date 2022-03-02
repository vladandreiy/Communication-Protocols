#include <string>
#include <vector>

#ifndef _REQUESTS_
#define _REQUESTS_

// computes and returns a GET request string (query_params, cookies
// optional headers can be null (vector<string()) if not needed)
char *compute_get_request(char *url, char *query_params,
                          const std::vector<std::string> &headers,
                          const std::vector<std::string> &cookies);

// computes and returns a POST request string (cookies, optional headers,
// body data can be null (vector<string>()) if not needed)
char *compute_post_request(char *host, char *url, char *content_type,
                           const std::vector<std::string> &headers,
                           const std::vector<std::string> &body_data,
                           const std::vector<std::string> &cookies);

// computes and returns a DELETE request string (cookies, optional headers,
// can be null (vector<string>()) if not needed)
char *compute_delete_request(char *url, char *query_params,
                             const std::vector<std::string> &headers,
                             const std::vector<std::string> &cookies);

#endif
