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

bool ConnectionManager::establishSSHTunnel(const char* identityFile, int localPort, const char* remoteHost, int remotePort, const char* sshServer, const char* sshUser) {
    ssh_session sshSession = ssh_new();

    if (!sshSession) {
        std::cerr << "Error al crear la sesión SSH" << std::endl;
        return false;
    }

    // Establecer opciones de conexión SSH
    ssh_options_set(sshSession, SSH_OPTIONS_HOST, sshServer);
    char portString[16];  // Espacio para almacenar el número de puerto como cadena
    snprintf(portString, sizeof(portString), "%d", remotePort);  // Convierte el puerto a una cadena
    ssh_options_set(sshSession, SSH_OPTIONS_PORT_STR, portString);

    // Establecer la clave privada para la autenticación
    ssh_options_set(sshSession, SSH_OPTIONS_IDENTITY, identityFile);

    // Conectar al servidor SSH
    if (ssh_connect(sshSession) != SSH_OK) {
        std::cerr << "Error al conectar al servidor SSH" << std::endl;
        ssh_free(sshSession);
        return false;
    }

    // Autenticarse en el servidor SSH
    if (ssh_userauth_publickey(sshSession, sshUser, NULL) != SSH_AUTH_SUCCESS) {
        std::cerr << "Error de autenticación SSH" << std::endl;
        ssh_disconnect(sshSession);
        ssh_free(sshSession);
        return false;
    }

    // Crear un canal SSH para el túnel
    ssh_channel channel = ssh_channel_new(sshSession);
    if (!channel) {
        std::cerr << "Error al crear el canal SSH" << std::endl;
        ssh_disconnect(sshSession);
        ssh_free(sshSession);
        return false;
    }

    // Abre el túnel SSH
    if (ssh_channel_open_forward(channel, remoteHost, remotePort, "localhost", localPort) != SSH_OK) {
        std::cerr << "Error al abrir el túnel SSH" << std::endl;
        ssh_channel_free(channel);
        ssh_disconnect(sshSession);
        ssh_free(sshSession);
        return false;
    }

    // Esperar indefinidamente (puedes modificar esto según tus necesidades)
    while (1) {
        // Aquí puedes agregar lógica adicional o simplemente esperar
    }

    // Cerrar la sesión SSH (debería hacerse en una función separada o en el destructor)
    ssh_channel_send_eof(channel);
    ssh_channel_free(channel);
    ssh_disconnect(sshSession);
    ssh_free(sshSession);

    return true;
}