#include "P2PMenu.h"

const Fl_Color green = 79;
const std::string authorizedKeysFile = "/home/ClientP2P/.ssh/authorized_keys";
const std::string ip = "https://p2pProxyService.com";
const char *serverCertPath = "/home/ClientP2P/utils/server/cert.pem";
const char* clientPath = "/home/ClientP2P/utils/client/certificado.pem";
const char* privateKeyPath = "/home/ClientP2P/utils/client/clave-privada.pem";
std::string randintHex = "";
std::string jwtClient = "";
std::atomic<bool> shouldRun(true);

bool containsSubstring(const std::string &line, const std::string &substring) {
    return line.find(substring) != std::string::npos;
}

bool fileExists(const char* filename) {
    std::ifstream file(filename);
    return file.good();
}

void generateCertificates() {
    std::cout << "Generando certificado y clave privada..." << std::endl;
    const char* command = "openssl req -x509 -newkey rsa:4096 -keyout /home/ClientP2P/utils/client/clave-privada.pem -out /home/ClientP2P/utils/client/certificado.pem -days 365 -nodes -subj \"/\"";
    int result = std::system(command);
    if (result != 0) {
        std::cerr << "Error al ejecutar el comando de generación de certificado y clave privada." << std::endl;
        // Puedes manejar el error de alguna manera apropiada, como lanzar una excepción o salir del programa.
    }
}

bool esClavePublicaSSH(const std::string& mensaje) {
    // Comprueba si el mensaje comienza con "ssh-"
    return mensaje.find("ssh-") == 0;
}

void P2PMenu::listenForConnections() {
    try {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        std::cout <<"Hello from thread" << std::endl;
        if (serverSocket == -1) {
            perror("Error al crear el socket del servidor");
            return;
        }

        struct sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(12345); // Puerto que deseas escuchar

        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
            perror("Error al vincular el socket del servidor");
            close(serverSocket);
            return;
        }

        if (listen(serverSocket, 5) == -1) {
            perror("Error al escuchar en el socket del servidor");
            close(serverSocket);
            return;
        }

        SSL_CTX* sslContext = SSL_CTX_new(SSLv23_server_method());
        if (!sslContext) {
            perror("Error al crear el contexto SSL");
            close(serverSocket);
            return;
        }

        if (SSL_CTX_use_certificate_file(sslContext, clientPath, SSL_FILETYPE_PEM) <= 0 ||
            SSL_CTX_use_PrivateKey_file(sslContext, privateKeyPath, SSL_FILETYPE_PEM) <= 0) {
            perror("Error al cargar el certificado o la clave privada");
            close(serverSocket);
            SSL_CTX_free(sslContext);
            return;
        }

        while (shouldRun.load()) {
            struct sockaddr_in clientAddress;
            socklen_t clientAddressLength = sizeof(clientAddress);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
            if (clientSocket == -1) {
                perror("Error al aceptar la conexión del cliente");
                continue;
            }

            SSL* ssl = SSL_new(sslContext);
            SSL_set_fd(ssl, clientSocket);
            if (SSL_accept(ssl) <= 0) {
                ERR_print_errors_fp(stderr); // Imprimir información detallada sobre el error SSL
                perror("Error en el proceso de aceptación SSL");
                close(clientSocket);
                SSL_free(ssl);
                continue;
            }
            // Aquí puedes manejar la conexión SSL entrante, leer y escribir datos con el cliente, etc.
            char buffer[2048]; // Un búfer para almacenar los datos recibidos
            ssize_t bytesRead;

            while ((bytesRead = SSL_read(ssl, buffer, sizeof(buffer))) > 0) {
                // Imprime lo que se recibe en la consola
                std::cout << "Mensaje recibido del cliente: " << std::string(buffer, bytesRead) << std::endl;
                std::string receivedData(buffer, bytesRead);
                // Parsear el JSON recibido
                Json::Value root;
                Json::CharReaderBuilder reader;
                std::istringstream jsonStream(receivedData);
                std::string errs;
                if (Json::parseFromStream(reader, jsonStream, &root, &errs)) {
                    if ((root.isMember("randint"))&&(root["randint"].asString()==randintHex)) {
                        if (root.isMember("publicKey")){
                            std::string publicKey = root["publicKey"].asString();
                            bool ssh = esClavePublicaSSH(publicKey);
                            if (ssh){
                                std::cout << "Succesfull detecterd public key" << std::endl;
                                storePublicKey(std::string(buffer,bytesRead),authorizedKeysFile);
                            }else{
                                std::cout << "Thats not public key" << std::endl;
                                printToConsole(std::string(buffer,bytesRead));
                            }
                        }
                        continue;
                    } else {
                        std::cout << "Someone is impersonatning server!!!" << std::endl;
                    }
                }
                /*
                Json::CharReaderBuilder readerBuilder;
                Json::CharReader* reader = readerBuilder.newCharReader();
                Json::Value jsonData;
                std::string errs;
                bool parsingSuccessful = Json::parseFromStream(reader, receivedData, &jsonData, &errs);
                if (parsingSuccessful) {
                    // Verificar si el JSON contiene el campo "publicKey" y "randint"
                    if (jsonData.isMember("publicKey") && jsonData.isMember("randint")) {
                        std::string publicKey = jsonData["publicKey"].asString();
                        std::string randintHexRecived = jsonData["randint"].asString();

                        if (randintHexRecived != randintHex){
                            std::cout<<"Someone is impersonating server";
                            break;
                        }else{
                            bool ssh = esClavePublicaSSH(std::string(buffer,bytesRead));
                            if (ssh){
                                storePublicKey(std::string(buffer,bytesRead),authorizedKeysFile);
                            }else{
                                printToConsole(std::string(buffer,bytesRead));
                            }
                        }
                    } else {
                        std::cerr << "El JSON no contiene los campos esperados." << std::endl;
                    }
                } else {
                    std::cerr << "Error al parsear el JSON: " << errs << std::endl;
                }
                }
                */
            if (bytesRead == -1) {
                perror("Error al recibir datos del cliente");
            }

            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(clientSocket);
        }

        SSL_CTX_free(sslContext);
        close(serverSocket);
    }
    } catch (const std::exception& e) {
        // Maneja la excepción aquí, por ejemplo, registrándola o imprimiéndola
        std::cerr << "Excepción en listenForConnections: " << e.what() << std::endl;
    } catch (...) {
        // Maneja otras excepciones no identificadas aquí
        std::cerr << "Excepción no identificada en listenForConnections" << std::endl;
    }
}


void P2PMenu::printToConsole(const std::string &message) {
    std::time_t now = std::time(nullptr);
    std::tm timeInfo = *std::localtime(&now);
    
    char timeStr[9];
    std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeInfo);
    
    std::string formattedMessage = "[" + std::string(timeStr) + "] " + message;
    
    Fl_Text_Buffer *buffer = consoleDisplay->buffer();
    buffer->append(formattedMessage.c_str());
    
    buffer->append("\n");
    consoleDisplay->scroll(consoleDisplay->h(), 0);
}

void P2PMenu::HideOffer(){
    this->buttonOffer->hide();
    this->proxyOfferText->hide();
    this->unsubscribeOffer->show();
    this->proxyUnsubscribeText->show();
    this->window->redraw();
}

void P2PMenu::ShowOffer(){
    this->buttonOffer->show();
    this->proxyOfferText->show();
    this->unsubscribeOffer->hide();
    this->proxyUnsubscribeText->hide();
    this->window->redraw();
    shouldRun.store(false);
}

void P2PMenu::HideProxy(){
    std::cout <<"Hiding proxy" << std::endl;
    this->clientButton->hide();
    this->assignProxy->hide();
    this->inputArray[0].form1->show();
    this->inputArray[0].form2->show();
    this->inputArray[0].form3->show();
    this->inputArray[0].establishTunnel->show();
    this->inputArray[0].disconnect->show();
    this->unassignProxyButton->show();
    this->unassignProxytext->show();
    this->showIpProxy = "Proxy Ip Address: " + this->ipProxy;
    this->textipProxy->label((this->showIpProxy.c_str()));
    this->textipProxy->labelfont(FL_BOLD | FL_ITALIC);
    this->textipProxy->show();
    this->roundButton->show();
    this->roundButtonSubstract->show();
    this->window->redraw();
}

void P2PMenu::ShowProxy(){
    this->textipProxy->hide();
    this->inputArray[this->numConnections].form1->hide();
    this->inputArray[this->numConnections].form2->hide();
    this->inputArray[this->numConnections].form3->hide();
    this->unassignProxytext->hide();
    this->clientButton->show();
    this->assignProxy->show();
    this->unassignProxyButton->hide();
    this->roundButton->hide();
    this->roundButtonSubstract->hide();
    //BUCLE
    for (int i=0; i <= numConnections; i++){
        this->inputArray[i].form1->hide();
        this->inputArray[i].form2->hide();
        this->inputArray[i].form3->hide();
        this->inputArray[i].establishTunnel->hide();
        this->inputArray[i].disconnect->hide();
    }
    this->numConnections=0;
    this->window->redraw();
}

static void static_clearButtonCallback(Fl_Widget *widget, void *data) {
    P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
    p2pMenu->clearButtonCallback();
}

void P2PMenu::clearButtonCallback() {
    consoleBuffer->text(""); // Limpia el buffer de la consola
    consoleDisplay->redraw();
}

bool P2PMenu::isPublicKeyPresent(const std::string &substringToCheck, const std::string &authorizedKeysFile) {
    std::ifstream file(authorizedKeysFile);
    if (!file) {
        //std::cerr << "Error al abrir el archivo: " << authorizedKeysFile << std::endl;
        printToConsole("Error abriendo el archivo authorized_keys");
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (containsSubstring(line, substringToCheck)) {
            return true; // La clave pública está presente
        }
    }
    printToConsole("Server key Not found, making request to server");
    return false; // La clave pública no se encontró en authorized_keys
}

bool P2PMenu::areKeysPresent() {
    std::ifstream id_rsa("./.ssh/id_rsa");
    if (!id_rsa) {
        printToConsole("Error abriendo el archivo id_rsa");
        id_rsa.close();
        return false;
    }
    else{
        id_rsa.close();
        std::ifstream id_rsa_pub("./.ssh/id_rsa.pub");
        if (!id_rsa_pub){
            printToConsole("Error abriendo el archivo id_rsa_pub");
            id_rsa_pub.close();
            return false;
        }
        id_rsa_pub.close();
        return true;
    }
}

bool P2PMenu::storePublicKey(const std::string pubKey ,const std::string &authorizedKeysFile){
    std::ofstream outFile(authorizedKeysFile, std::ios::app);
    if (!outFile) {
        std::cerr << "Error al abrir el archivo: " << authorizedKeysFile << std::endl;
        return false; // Termina el programa con código de error
    }
    outFile << pubKey; // Añade la cadena al archivo
    outFile.close();
    return true;
}

void P2PMenu::buttonOfferCallback(Fl_Widget *widget, void *data) {
    std::cout << "Aquí llegamos 3" << std::endl;
    try {
        std::string substringToCheck = "P2PServerKey"; // Subcadena a buscar
        bool keyIsPresent = isPublicKeyPresent(substringToCheck,authorizedKeysFile);
        if (!keyIsPresent){
            const std::string url_key = ip + ":3000/get-server-public-key";
            const char *url_key_cstr = url_key.c_str();
            std::string res;
            if (this->connectionManager->postRequest(url_key_cstr, "",res)){
                printToConsole("Respuesta del servidor: " + res);
                Json::Value root;
                Json::CharReaderBuilder reader;
                std::istringstream jsonStream(res);
                std::string errs;
                if (Json::parseFromStream(reader, jsonStream, &root, &errs)) {
                    if (root.isMember("serverPublicKey")) {
                        std::string publicKey = root["serverPublicKey"].asString();
                        std::cout << "Public Key: " << publicKey << std::endl;
                        bool stored = storePublicKey(publicKey,authorizedKeysFile);
                        if (!stored){
                            printToConsole("Error storing key, check file " + authorizedKeysFile);
                        }
                    } else {
                        std::cerr << "No 'serverPublicKey' found in JSON." << std::endl;
                    }
                } else {
                    std::cerr << "JSON parse error: " << errs << std::endl;
                }
            }else{
                printToConsole("Error retriving server pub key: " + res);
            }
        }

        if (!fileExists(clientPath) || !fileExists(privateKeyPath)) {
            generateCertificates();
        }
        std::ifstream inputFile(clientPath);
        if (!inputFile.is_open()) {
            std::cerr << "No se pudo abrir el archivo." << std::endl;
        }
        std::stringstream buffer;
        buffer << inputFile.rdbuf(); // Lee el contenido del archivo en el buffer
        std::string content = buffer.str(); // Guarda el contenido en una variable
        inputFile.close(); // Cierra el archivo

        std::cout <<"PUBLIC CERT PEM:" << content <<std::endl;

        Json::Value jsonData;
        jsonData["certificatePem"] = content;

        //Generar el numero aleatorio
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;
        // Generar un número aleatorio de 256 bits
        uint64_t randomValue[4];
        for (int i = 0; i < 4; ++i) {
            randomValue[i] = dis(gen);
        }
        //Convertir el numero a hex
        std::ostringstream oss;
        for (int i = 0; i < 4; ++i) {
            oss << std::hex << std::setw(16) << std::setfill('0') << randomValue[i];
        }
        std::string randomNumberHex = oss.str();
        randintHex = randomNumberHex;
        jsonData["randint"] = randomNumberHex;
        std::cout << "Random num hex: " << randomNumberHex << std::endl;
        Json::StreamWriterBuilder writer;
        std::string jsonDataStr = Json::writeString(writer, jsonData);

        const std::string url = ip + ":3000/subscribe-proxy";
        const char *url_cstr = url.c_str();

        std::string response;

        // Configura los encabezados adecuados, incluido el Content-Type
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        ConnectionManager connectionManager(serverCertPath);
        try {
            std::thread listenerThread(&P2PMenu::listenForConnections, this);
            listenerThread.detach();
        }catch (const std::exception& e) {
            // Maneja la excepción aquí, por ejemplo, registrándola o imprimiéndola
            printToConsole("Excepción en listenForConnections: " + std::string(e.what()));
        } catch (...) {
            // Maneja otras excepciones no identificadas aquí
            printToConsole("Excepción no identificada en listenForConnections");
        }

        if (connectionManager.postRequestWithData(url_cstr, jsonDataStr.c_str(), response, headers)) {
            printToConsole("Respuesta del servidor: " + response);
            // Parsear la respuesta JSON
            Json::CharReaderBuilder reader;
            std::istringstream responseStream(response);
            Json::Value jsonResponse;
            Json::parseFromStream(reader, responseStream, &jsonResponse, nullptr);

             // Verificar si la respuesta contiene la clave 'token'
            if (jsonResponse.isMember("tokenProxy")) {
                // Obtener el valor del token
                std::string token = jsonResponse["tokenProxy"].asString();
                //std::cout << "Token recibido del servidor: " << token << std::endl;
                this->connectionManager->setJWT(token);

                // Ahora 'token' contiene el valor del token
                // Puedes hacer lo que necesites con el token en tu aplicación
            } else {
                std::cout << "La respuesta no contiene la clave 'token', tiene:" << jsonResponse << std::endl;
            }

            this->Circle->color(FL_GREEN);
        } else {
            std::cout << "Error en la respuesta del servidor???" << std::endl;
        }
    } catch (...) {
        std::cerr << "Excepción no identificada atrapada" << std::endl;
    }
}

void P2PMenu::buttonUnsubscribeCallback(Fl_Widget *widget, void *data) {
    try {
        //std::string substringToCheck = "P2PServerKey"; // Subcadena a buscar
        //std::string authorizedKeysFile = "/home/ClientP2P/.ssh/authorized_keys";
        //isPublicKeyPresent(substringToCheck,authorizedKeysFile);
        const std::string url = ip + ":3000/unsubscribe-proxy";
        const char *url_cstr = url.c_str();
        std::string response;
        
        if (this->connectionManager->postRequestWithJWT(url_cstr, "", response)) {
            //std::cout << "Respuesta del servidor: " << response << std::endl;
            printToConsole("Respuesta del servidor: " + response);
            this->proxyUnsubscribeText->hide();
            this->unsubscribeOffer->hide();
            this->buttonOffer->show();
            this->proxyOfferText->show();
            this->Circle->color(FL_RED);
        } else {
            std::cout << "Error en la respuesta del servidor???" << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "Excepción atrapada: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Excepción no identificada atrapada" << std::endl;
    }
}

std::string P2PMenu::askForProxy(Fl_Widget *widget, void *data) {
    try {
        const char *sshDirPath = "./.ssh"; // Ruta al directorio ./.ssh
        if (system(("mkdir -p " + std::string(sshDirPath)).c_str()) == 0) {
            std::cout << "Directorio ./.ssh configurado exitosamente." << std::endl;
        } else {
            std::cerr << "Error al crear el directorio ./.ssh." << std::endl;
        }
        //std::string substringToCheck = "P2PServerKey"; // Subcadena a buscar
        //std::string authorizedKeysFile = "/home/ClientP2P/.ssh/authorized_keys";
        bool keys = areKeysPresent();
        if (!keys){
            std::string keyPath = "./.ssh/";
            std::string command = "ssh-keygen -t rsa -b 4096 -N '' -f " + keyPath + "id_rsa";

            int result = system(command.c_str());
            if (result == 0) {
                std::cout << "Las claves se generaron exitosamente." << std::endl;
            } else {
                std::cerr << "No se pudieron generar las claves." << std::endl;
            }
        }

        std::ifstream inputFile("./.ssh/id_rsa.pub");
        if (!inputFile.is_open()) {
            std::cerr << "No se pudo abrir el archivo." << std::endl;
        }
        std::stringstream buffer;
        buffer << inputFile.rdbuf(); // Lee el contenido del archivo en el buffer
        std::string content = buffer.str(); // Guarda el contenido en una variable
        inputFile.close(); // Cierra el archivo

        std::cout <<"PUBLIC KEY:" << content <<std::endl;

        Json::Value jsonData;
        jsonData["public_key_content"] = content;

        Json::StreamWriterBuilder writer;
        std::string jsonDataStr = Json::writeString(writer, jsonData);

        const std::string url = ip + ":3000/assign_proxy";
        const char *url_cstr = url.c_str();

        std::string response;

        // Configura los encabezados adecuados, incluido el Content-Type
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        ConnectionManager connectionManager(serverCertPath);

        if (connectionManager.postRequestWithData(url_cstr, jsonDataStr.c_str(), response, headers)) {
            printToConsole("Respuesta del servidor: " + response);
            // Analizar la respuesta JSON
            Json::Value root;
            Json::CharReaderBuilder reader;
            std::istringstream responseStream(response);
            std::string errors;
            Json::parseFromStream(reader, responseStream, &root, &errors);

            // Extraer la dirección IP
            std::string ipAddress;

            if (!errors.empty()) {
                std::cerr << "Error al analizar la respuesta JSON: " << errors << std::endl;
            }

            if (root.isMember("message")) {
                std::string ipAddress = root["message"].asString();
                std::string searchString = "Proxy ip: ";
                size_t found = ipAddress.find(searchString);
                std::cout << "Value ipAddress: " << ipAddress << std::endl;

                // Verificar si se encontró la cadena de búsqueda
                if (found != std::string::npos) {
                    std::cout << "found: " << found << std::endl;
                    ipAddress = ipAddress.substr(found + searchString.length());
                    if (root.isMember("tokenClient")) {
                        jwtClient = root["tokenClient"].asString();
                    } else {
                        std::cerr << "No se encontró el campo 'tokenClient' en la respuesta JSON." << std::endl;
                    }
                    return ipAddress;
                } else {
                    std::cerr << "No se encontró la cadena 'Proxy ip: ' en la respuesta." << std::endl;
                }
            } else {
                std::cerr << "No se encontró el campo 'message' en la respuesta JSON." << std::endl;
            }
        } else {
            printToConsole("Respuesta del servidor: " + response);
            return "";
        }
    } catch (const std::exception &e) {
        std::cerr << "Excepción atrapada: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Excepción no identificada atrapada" << std::endl;
    }
}

void P2PMenu::unassignProxy(Fl_Widget *widget, void *data){
    try{
        const std::string url = ip + ":3000/unassign_proxy";
        const char *url_cstr = url.c_str();
        std::string response;
        ConnectionManager connectionManager(serverCertPath,jwtClient);
        if (connectionManager.postRequestWithJWT(url_cstr ,"",response)) {
            printToConsole("Respuesta del servidor: " + response);
        } else {
            std::cout << "Error en la respuesta del servidor." << std::endl;
        }
    }catch (const std::exception &e) {
        std::cerr << "Excepción atrapada: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Excepción no identificada atrapada" << std::endl;
    }
}

void P2PMenu::initRoundButton(int x, int y, int w, int h, const char *label) {
    this->numConnections = 1;
    this->roundButton = new RoundButton(x, y, w, h, "+"); // Crea un botón redondo con un símbolo de línea horizontal
    this->roundButtonSubstract = new RoundButton(x + 30, y, w, h, "-"); 
    this->roundButtonSubstract->color(FL_RED);
    this->roundButton->hide();  // Muestra el botón en la interfaz
    this->roundButtonSubstract->hide();
}

int P2PMenu::getNumConnections(){
    return this->numConnections;
}

ConnectionManager* P2PMenu::getConnectionManager(){
    return this->connectionManager;
}

std::string P2PMenu::getIpProxy(){
    return this->ipProxy;
}

InputGroup P2PMenu::getInputIndex(int index){
    return this->inputArray[index];
}

void P2PMenu::blockSession(int index){
    this->inputArray[index].establishTunnel->deactivate();
    this->inputArray[index].disconnect->activate();
}

void P2PMenu::unlockSession(int index){
    this->inputArray[index].form1->value("");
    this->inputArray[index].form2->value("");
    this->inputArray[index].form3->value("");
    this->inputArray[index].establishTunnel->activate();
    this->inputArray[index].disconnect->deactivate();
}

void P2PMenu::drawInputs(){
    this->numConnections = 0;
    this->inputArray[0].form1 = new Fl_Input(700, 85, 200, 30, "Session 1:");
    this->inputArray[0].form2 = new Fl_Input(925, 85, 50, 30, "");
    this->inputArray[0].form3 = new Fl_Input(1000, 85, 50, 30, "");
    this->inputArray[0].establishTunnel = new Fl_Button(1065, 85, 70, 30, "connect");
    this->inputArray[0].establishTunnel->color(FL_GREEN);
    this->inputArray[0].disconnect = new Fl_Button(1145, 85, 80, 30, "disconnect");
    this->inputArray[0].disconnect->color(FL_RED);
    this->inputArray[0].disconnect->deactivate();
    this->inputArray[0].establishTunnel->callback([](Fl_Widget *widget, void *data) {
        P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
        InputGroup values = p2pMenu->getInputIndex(0);
        if ((strlen(values.form1->value()) != 0)&&(strlen(values.form2->value()) != 0)&&(strlen(values.form3->value()) != 0)) {
            try {
                int remotePort = std::stoi(values.form2->value());
                std::cout << "Entero: " << remotePort << std::endl;
                try {
                    int localPort = std::stoi(values.form3->value());
                    std::cout << "Entero: " << localPort << std::endl;
                    bool opened = p2pMenu->getConnectionManager()->establishSSHTunnel("./.ssh/id_rsa",values.form1->value(),"ClientP2P",p2pMenu->getIpProxy().c_str(),remotePort,localPort,0);
                    if (opened){
                        p2pMenu->blockSession(0);
                    }
                } catch (const std::invalid_argument& e) {
                    std::cout << "La cadena no es un número válido." << std::endl;
                } catch (const std::out_of_range& e) {
                    std::cout << "El número está fuera del rango de representación de int." << std::endl;
                }
            } catch (const std::invalid_argument& e) {
                std::cout << "La cadena no es un número válido." << std::endl;
            } catch (const std::out_of_range& e) {
                std::cout << "El número está fuera del rango de representación de int." << std::endl;
            }
        }
    }, this);
    this->inputArray[0].disconnect->callback([](Fl_Widget *widget, void * data) {
        P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
        bool closed = p2pMenu->getConnectionManager()->deleteSSHTunnel(0);
        if (closed){
            p2pMenu->unlockSession(0);
        }
    },this);


    this->inputArray[0].form1->hide();
    this->inputArray[0].form2->hide();
    this->inputArray[0].form3->hide();
    this->inputArray[0].establishTunnel->hide();
    this->inputArray[0].disconnect->hide();

    this->inputArray[1].form1 = new Fl_Input(700, 125, 200, 30, "Session 2:");
    this->inputArray[1].form2 = new Fl_Input(925, 125, 50, 30, "");
    this->inputArray[1].form3 = new Fl_Input(1000, 125, 50, 30, "");
    this->inputArray[1].establishTunnel = new Fl_Button(1065, 125, 70, 30, "connect");
    this->inputArray[1].establishTunnel->color(FL_GREEN);
    this->inputArray[1].disconnect = new Fl_Button(1145, 125, 80, 30, "disconnect");
    this->inputArray[1].disconnect->color(FL_RED);
    this->inputArray[1].disconnect->deactivate();
    this->inputArray[1].establishTunnel->callback([](Fl_Widget *widget, void *data) {
        P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
        InputGroup values = p2pMenu->getInputIndex(1);
        if ((strlen(values.form1->value()) != 0)&&(strlen(values.form2->value()) != 0)&&(strlen(values.form3->value()) != 0)) {
            try {
                int remotePort = std::stoi(values.form2->value());
                std::cout << "Entero: " << remotePort << std::endl;
                try {
                    int localPort = std::stoi(values.form3->value());
                    std::cout << "Entero: " << localPort << std::endl;
                    bool opened = p2pMenu->getConnectionManager()->establishSSHTunnel("./.ssh/id_rsa",values.form1->value(),"ClientP2P",p2pMenu->getIpProxy().c_str(),remotePort,localPort,1);
                    if (opened){
                        p2pMenu->blockSession(1);
                    }
                } catch (const std::invalid_argument& e) {
                    std::cout << "La cadena no es un número válido." << std::endl;
                } catch (const std::out_of_range& e) {
                    std::cout << "El número está fuera del rango de representación de int." << std::endl;
                }
            } catch (const std::invalid_argument& e) {
                std::cout << "La cadena no es un número válido." << std::endl;
            } catch (const std::out_of_range& e) {
                std::cout << "El número está fuera del rango de representación de int." << std::endl;
            }
        }
    }, this);
    this->inputArray[1].disconnect->callback([](Fl_Widget *widget, void * data) {
        P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
        bool closed = p2pMenu->getConnectionManager()->deleteSSHTunnel(1);
        if (closed){
            p2pMenu->unlockSession(1);
        }
    },this);

    this->inputArray[1].form1->hide();
    this->inputArray[1].form2->hide();
    this->inputArray[1].form3->hide();
    this->inputArray[1].establishTunnel->hide();
    this->inputArray[1].disconnect->hide();

    this->inputArray[2].form1 = new Fl_Input(700, 165, 200, 30, "Session 3:");
    this->inputArray[2].form2 = new Fl_Input(925, 165, 50, 30, "");
    this->inputArray[2].form3 = new Fl_Input(1000, 165, 50, 30, "");
    this->inputArray[2].establishTunnel = new Fl_Button(1065, 165, 70, 30, "connect");
    this->inputArray[2].establishTunnel->color(FL_GREEN);
    this->inputArray[2].disconnect = new Fl_Button(1145, 165, 80, 30, "disconnect");
    this->inputArray[2].disconnect->color(FL_RED);
    this->inputArray[2].disconnect->deactivate();
    this->inputArray[2].establishTunnel->callback([](Fl_Widget *widget, void *data) {
        P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
        InputGroup values = p2pMenu->getInputIndex(2);
        if ((strlen(values.form1->value()) != 0)&&(strlen(values.form2->value()) != 0)&&(strlen(values.form3->value()) != 0)) {
            try {
                int remotePort = std::stoi(values.form2->value());
                std::cout << "Entero: " << remotePort << std::endl;
                try {
                    int localPort = std::stoi(values.form3->value());
                    std::cout << "Entero: " << localPort << std::endl;
                    bool opened = p2pMenu->getConnectionManager()->establishSSHTunnel("./.ssh/id_rsa",values.form1->value(),"ClientP2P",p2pMenu->getIpProxy().c_str(),remotePort,localPort,2);
                    if (opened){
                        p2pMenu->blockSession(2);
                    }
                } catch (const std::invalid_argument& e) {
                    std::cout << "La cadena no es un número válido." << std::endl;
                } catch (const std::out_of_range& e) {
                    std::cout << "El número está fuera del rango de representación de int." << std::endl;
                }
            } catch (const std::invalid_argument& e) {
                std::cout << "La cadena no es un número válido." << std::endl;
            } catch (const std::out_of_range& e) {
                std::cout << "El número está fuera del rango de representación de int." << std::endl;
            }
        }
    }, this);
    this->inputArray[2].disconnect->callback([](Fl_Widget *widget, void * data) {
        P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
        bool closed = p2pMenu->getConnectionManager()->deleteSSHTunnel(2);
        if (closed){
            p2pMenu->unlockSession(2);
        }
    },this);

    this->inputArray[2].form1->hide();
    this->inputArray[2].form2->hide();
    this->inputArray[2].form3->hide();
    this->inputArray[2].establishTunnel->hide();
    this->inputArray[2].disconnect->hide();

    this->inputArray[3].form1 = new Fl_Input(700, 205, 200, 30, "Session 4:");
    this->inputArray[3].form2 = new Fl_Input(925, 205, 50, 30, "");
    this->inputArray[3].form3 = new Fl_Input(1000, 205, 50, 30, "");
    this->inputArray[3].establishTunnel = new Fl_Button(1065, 205, 70, 30, "connect");
    this->inputArray[3].establishTunnel->color(FL_GREEN);
    this->inputArray[3].disconnect = new Fl_Button(1145, 205, 80, 30, "disconnect");
    this->inputArray[3].disconnect->color(FL_RED);
    this->inputArray[3].disconnect->deactivate();
    this->inputArray[3].establishTunnel->callback([](Fl_Widget *widget, void *data) {
        P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
        InputGroup values = p2pMenu->getInputIndex(3);
        if ((strlen(values.form1->value()) != 0)&&(strlen(values.form2->value()) != 0)&&(strlen(values.form3->value()) != 0)) {
            try {
                int remotePort = std::stoi(values.form2->value());
                std::cout << "Entero: " << remotePort << std::endl;
                try {
                    int localPort = std::stoi(values.form3->value());
                    std::cout << "Entero: " << localPort << std::endl;
                    bool opened = p2pMenu->getConnectionManager()->establishSSHTunnel("./.ssh/id_rsa",values.form1->value(),"ClientP2P",p2pMenu->getIpProxy().c_str(),remotePort,localPort,3);
                    if (opened){
                        p2pMenu->blockSession(3);
                    }
                } catch (const std::invalid_argument& e) {
                    std::cout << "La cadena no es un número válido." << std::endl;
                } catch (const std::out_of_range& e) {
                    std::cout << "El número está fuera del rango de representación de int." << std::endl;
                }
            } catch (const std::invalid_argument& e) {
                std::cout << "La cadena no es un número válido." << std::endl;
            } catch (const std::out_of_range& e) {
                std::cout << "El número está fuera del rango de representación de int." << std::endl;
            }
        }
    }, this);
    this->inputArray[3].disconnect->callback([](Fl_Widget *widget, void * data) {
        P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
        bool closed = p2pMenu->getConnectionManager()->deleteSSHTunnel(3);
        if (closed){
            p2pMenu->unlockSession(3);
        }
    },this);

    this->inputArray[3].form1->hide();
    this->inputArray[3].form2->hide();
    this->inputArray[3].form3->hide();
    this->inputArray[3].establishTunnel->hide();
    this->inputArray[3].disconnect->hide();
}

void P2PMenu::substractConnection(){
    this->inputArray[this->numConnections].form1->hide();
    this->inputArray[this->numConnections].form2->hide();
    this->inputArray[this->numConnections].form3->hide();
    this->inputArray[this->numConnections].form1->value("");
    this->inputArray[this->numConnections].form2->value("");
    this->inputArray[this->numConnections].form3->value("");
    this->inputArray[this->numConnections].establishTunnel->hide();
    this->inputArray[this->numConnections].disconnect->hide();
    this->numConnections += -1;
    this->window->redraw();
}

void P2PMenu::addConnection(){
    this->numConnections += 1;
    this->inputArray[this->numConnections].form1->show();
    this->inputArray[this->numConnections].form2->show();
    this->inputArray[this->numConnections].form3->show();
    this->inputArray[this->numConnections].establishTunnel->show();
    this->inputArray[this->numConnections].disconnect->show();
    this->window->redraw();
}
P2PMenu::P2PMenu() {
    this->connectionManager = new ConnectionManager(serverCertPath);
    // Obtener el ancho y alto de la pantalla
    int screenWidth = Fl::w();
    int screenHeight = Fl::h();

    // Calcular las dimensiones de la ventana
    int windowWidth = screenWidth * 0.8; // Por ejemplo, el 80% del ancho
    int windowHeight = screenHeight * 0.8; // Por ejemplo, el 80% del alto

    //std::cout << "windowWidth: " << windowWidth << std::endl;
    //std::cout << "windowHeight: " << windowHeight << std::endl;

    //windowWidth: 1480
    //windowHeight: 733

    // Crear una ventana centrada en la pantalla
    int windowX = (screenWidth - windowWidth) / 2;
    int windowY = (screenHeight - windowHeight) / 2;
    this->window = new Fl_Window(windowWidth, windowHeight, "P2P Menu");
    this->window->position(windowX, windowY);

    // Cambiar el color de fondo de la ventana a un azul claro (RGB: 173, 216, 230)
    Fl::set_color(FL_BACKGROUND_COLOR, 173, 216, 230);
    window->color(FL_BACKGROUND_COLOR);

    //Crear texto encima del botón
    this->proxyOfferText = new Fl_Box(80, 45, 100, 20, "Ofrecerse como proxy");
    this->proxyOfferText->box(FL_NO_BOX); // Eliminar el borde del widget de texto
    this->proxyOfferText->color(FL_WHITE);

    //Crear texto encima del botón
    this->proxyUnsubscribeText = new Fl_Box(80, 45, 100, 20, "Stop serving as proxy");
    this->proxyUnsubscribeText->box(FL_NO_BOX); // Eliminar el borde del widget de texto
    this->proxyUnsubscribeText->color(FL_WHITE);
    this->proxyUnsubscribeText->hide();

    this->assignProxy = new Fl_Box(400,45,100,20,"Ask for Proxy");
    this->assignProxy->box(FL_NO_BOX);
    this->assignProxy->color(FL_WHITE);

    this->unassignProxytext = new Fl_Box(400,45,100,20,"UnassignProxy");
    this->unassignProxytext->box(FL_NO_BOX);
    this->unassignProxytext->color(FL_WHITE);
    this->unassignProxytext->hide();

    this->consoleText = new Fl_Box(675,385,100,20, "Console logs");
    this->consoleText->box(FL_NO_BOX);
    this->consoleText->color(FL_WHITE);

    this->servingStatus = new Fl_Box(1275,55,100,20, "Serving Status");
    this->servingStatus->box(FL_NO_BOX);
    this->servingStatus->color(FL_WHITE);

    // Crear un botón en la esquina superior izquierda
    this->buttonOffer = new Fl_Button(80, 85, 100, 30, "Haz clic");
    this->buttonOffer->color(green);
    this->buttonOffer->callback([](Fl_Widget *widget, void *data) {
    P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
    p2pMenu->buttonOfferCallback(widget, p2pMenu);
    p2pMenu->HideOffer();
    //Fl_Button *b = (Fl_Button *)widget;
    //b->hide();
    }, this);

    // Crear un botón en la esquina superior izquierda
    this->unsubscribeOffer = new Fl_Button(80, 85, 100, 30, "Haz clic");
    this->unsubscribeOffer->color(FL_RED);
    this->unsubscribeOffer->hide();
    this->unsubscribeOffer->callback([](Fl_Widget *widget, void *data) {
    P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
    p2pMenu->buttonUnsubscribeCallback(widget, p2pMenu);
    
    //Fl_Button *b = (Fl_Button *)widget;
    //b->hide();
    }, this);

    this->clientButton = new Fl_Button(400,85,100,30,"Haz clic");
    this->clientButton->color(FL_GREEN);
    this->clientButton->callback([](Fl_Widget *widget, void *data) {
    P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
    p2pMenu->ipProxy = p2pMenu->askForProxy(widget, p2pMenu);
    if (p2pMenu->ipProxy != ""){
        p2pMenu->HideProxy();
    }
    //Fl_Button *b = (Fl_Button *)widget;
    //b->hide();
    }, this);

    this->unassignProxyButton = new Fl_Button(400,85,100,30,"Haz clic");
    this->unassignProxyButton->color(FL_RED);
    this->unassignProxyButton->callback([](Fl_Widget *widget, void *data) {
    P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
    p2pMenu->unassignProxy(widget, p2pMenu);
    p2pMenu->ShowProxy();
    //Fl_Button *b = (Fl_Button *)widget;
    //b->hide();
    }, this);
    this->unassignProxyButton->hide();

    this->textipProxy = new Fl_Box(1130, 20, 400, 20, "");
    this->textipProxy->box(FL_NO_BOX);
    this->textipProxy->color(FL_WHITE);
    this->textipProxy->hide();

    this->Circle = new Fl_Box(1400, 50, 25, 25); // Tamaño del círculo
    this->Circle->box(FL_ROUND_DOWN_BOX); // Establece el estilo redondeado
    this->Circle->color(FL_RED); // Establece el color

    this->clearButton = new Fl_Button(1300, 380, 100, 30, "Clear");
    this->clearButton->color(green);
    this->clearButton->callback(static_clearButtonCallback, this);

    this->consoleBuffer = new Fl_Text_Buffer();
    this->consoleDisplay = new Fl_Text_Display(50, 420, windowWidth - 100, windowHeight - 450);
    this->consoleDisplay->buffer(consoleBuffer);
    this->consoleDisplay->textfont(FL_COURIER);

    this->initRoundButton(1050, 40, 50, 50, "Botón");
    this->roundButtonSubstract->callback([](Fl_Widget *widget, void *data) {
    P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
    if (p2pMenu->getNumConnections() != 0){
        p2pMenu->substractConnection();
    }
    }, this);
    this->roundButton->callback([](Fl_Widget *widget, void *data) {
    P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
    if (p2pMenu->getNumConnections() != 3){
        p2pMenu->addConnection();
    }
    }, this);
    this->drawInputs();
    this->window->end();
}

void P2PMenu::run() {
    this->window->show();
    Fl::run();
}