#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <curl/curl.h>
#include <libssh/libssh.h>
#include <string>
#include <iostream>

class ConnectionManager {
public:
    ConnectionManager();
    ~ConnectionManager();
    bool postRequest(const char *url, const char *postData, std::string &response);
    bool postRequestWithData(const char *url, const char *postData, std::string &response, struct curl_slist *headers);
    bool establishSSHTunnel(const char* identityFile, int localPort, const char* remoteHost, int remotePort, const char* sshServer, const char* sshUser);
private:
    CURL *curl;
};

#endif // CONNECTION_MANAGER_H