#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <curl/curl.h>
#include <libssh/libssh.h>
#include <string>
#include <iostream>
#include <cstring>
#include <ctime>

class ConnectionManager {
public:
    ConnectionManager(std::string certPath);
    ConnectionManager(std::string certPath, std::string JWT);
    ~ConnectionManager();
    bool postRequest(const char *url, const char *postData, std::string &response);
    bool postRequestWithJWT(const char *url, const char *postData, std::string &response);
    bool postRequestWithData(const char *url, const char *postData, std::string &response, struct curl_slist *headers);
    bool establishSSHTunnel(const char* identityFile,const char* remoteHost,const char* sshUser,const char* sshServer,int remotePort,int localPort, int index);
    bool deleteSSHTunnel(int index);
    void setJWT(std::string JWT);
private:
    std::string jwt;
    std::string certPath;
    int pids[4];
    CURL *curl;
};

#endif // CONNECTION_MANAGER_H