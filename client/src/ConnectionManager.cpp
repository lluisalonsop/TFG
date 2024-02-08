#include "ConnectionManager.h"
#include <fstream>

size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t totalSize = size * nmemb;
    std::string *response = static_cast<std::string *>(userp);
    response->append(static_cast<char *>(contents), totalSize);
    return totalSize;
}

ConnectionManager::ConnectionManager(std::string certPath) : certPath(certPath) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
}

ConnectionManager::ConnectionManager(std::string certPath, std::string JWT) : certPath(certPath), jwt(JWT) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
}


ConnectionManager::~ConnectionManager() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

bool ConnectionManager::postRequest(const char *url, const char *postData, std::string &response) {
    if (!curl) {
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Habilita la verificación del certificado
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L); // 2L para verificar el nombre del host

    // Establece la ruta del certificado
    curl_easy_setopt(curl, CURLOPT_CAINFO, certPath.c_str());

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Error en la solicitud POST: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    std::cout << "Aquí llegamos 5" << std::endl;
    return true;
}

bool ConnectionManager::postRequestWithJWT(const char *url, const char *postData, std::string &response) {
    if (!curl) {
        return false;
    }

    // Crear una lista para los encabezados
    struct curl_slist *headers = nullptr;

    std::string authorizationHeader = "Authorization: Bearer " + this->jwt;
    std::cout <<"Header: " <<authorizationHeader<<std::endl;
    headers = curl_slist_append(headers, authorizationHeader.c_str());

    // Establecer los encabezados en la solicitud
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Configurar otras opciones de la solicitud
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Habilita la verificación del certificado
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L); // 2L para verificar el nombre del host

    // Establece la ruta del certificado
    curl_easy_setopt(curl, CURLOPT_CAINFO, certPath.c_str());

    // Realizar la solicitud
    CURLcode res = curl_easy_perform(curl);

    // Limpiar la lista de encabezados después de la solicitud
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        std::cerr << "Error en la solicitud POST: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    std::cout << "Aquí llegamos 5" << std::endl;
    return true;
}

bool ConnectionManager::postRequestWithData(const char *url, const char *postData, std::string &response, struct curl_slist *headers) {
    if (!curl) {
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Configura los encabezados personalizados
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Habilita la verificación del certificado
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L); // 2L para verificar el nombre del host

    // Establece la ruta del certificado
    curl_easy_setopt(curl, CURLOPT_CAINFO, certPath.c_str());

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Error en la solicitud POST: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    if (httpCode == 503) {
        // El código de respuesta es 503 (Service Unavailable)
        return false;
    }
    return true;
}

bool ConnectionManager::establishSSHTunnel(const char* identityFile,const char* remoteHost,const char* sshUser,const char* sshServer,int remotePort,int localPort, int index) {
    //const char* command = "ssh -i ../client/.ssh/id_rsa -L 1337:edge-eu-starting-point-1-dhcp.hackthebox.eu:443 ClientP2P@192.168.137.240 -p 443 -N&";
    std::string command = "ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -i " + std::string(identityFile) + " -L " + std::to_string(localPort) + ":" + remoteHost + ":" + std::to_string(remotePort) + " " + sshUser + "@" + sshServer + " -p 443 -N&";
    std::cout << "Command: " << command << std::endl;
    int result = system(command.c_str());
    sleep(1);
    FILE *fp = popen("lsof -i :1337 -t", "r");
    if (fp) {
        char buffer[10];
        if (fgets(buffer, sizeof(buffer), fp) != nullptr) {
            int pid = std::atoi(buffer);
            std::cout << "PID del proceso en el puerto 1337: " << pid << std::endl;
            this->pids[index] = pid;
        }
        pclose(fp);
        return true;
    } else {
        std::cerr << "Error al ejecutar lsof" << std::endl;
    }

}

bool ConnectionManager::deleteSSHTunnel(int index){
    std::string command = "kill " + std::to_string(this->pids[index]);
    std::cout << "Command: " << command << std::endl;
    int result = system(command.c_str());
    if (result == 0) {
        std::cout << "Proceso con PID " << this->pids[index] << " terminado con éxito." << std::endl;
        return true;
    } else {
        std::cerr << "Error al intentar matar el proceso con PID " << this->pids[index] << std::endl;
        return false;
    }
}

void ConnectionManager::setJWT(std::string JWT){
    this->jwt = JWT;
}