#include <cstring>
#include <vector>

char *compute_get_request(const char *host, const char *url, char *query_params, char **cookies, int cookies_count, char *token);
char* post_request(const char *host, const char *url, const char *content_type, std::string body_data, std::vector<std::string> cookies , int cookies_num, char *token);
char *compute_delete_request(const char *host, const char *url, char *query_params,
                            char **cookies, int cookies_count, char *token);


