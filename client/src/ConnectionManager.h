#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <curl/curl.h>
#include <string>
#include <iostream>

class ConnectionManager {
public:
    ConnectionManager();
    ~ConnectionManager();
    bool postRequest(const char *url, const char *postData, std::string &response);

private:
    CURL *curl;
};

#endif // CONNECTION_MANAGER_H