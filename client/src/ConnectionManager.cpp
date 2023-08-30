#include "ConnectionManager.h"

size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t totalSize = size * nmemb;
    std::string *response = static_cast<std::string *>(userp);
    response->append(static_cast<char *>(contents), totalSize);
    return totalSize;
}

ConnectionManager::ConnectionManager() {
    curl = curl_easy_init();
}

ConnectionManager::~ConnectionManager() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

bool ConnectionManager::postRequest(const char *url, const char *postData, std::string &response) {
    std::cout << "Aquí llegamos 4" << std::endl;
    if (!curl) {
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Error en la solicitud POST: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    std::cout << "Aquí llegamos 5" << std::endl;
    return true;
}

bool ConnectionManager::postRequestWithData(const char *url, const char *postData, std::string &response, struct curl_slist *headers) {
    std::cout << "Aquí llegamos 4" << std::endl;
    if (!curl) {
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Configura los encabezados personalizados
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Error en la solicitud POST: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    std::cout << "Aquí llegamos 5" << std::endl;
    return true;
}