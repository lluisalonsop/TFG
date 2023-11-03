#include "P2PMenu.h"

const Fl_Color green = 79;
const std::string authorizedKeysFile = "/home/ClientP2P/.ssh/authorized_keys";
const std::string ip = "http://192.168.137.234";

std::atomic<bool> shouldRun(true);

bool containsSubstring(const std::string &line, const std::string &substring) {
    return line.find(substring) != std::string::npos;
}

/*void P2PMenu::printToConsole(const std::string &message) {
    this->consoleBuffer->append(message.c_str());
    this->consoleBuffer->append("\n");
}
*/

bool esClavePublicaSSH(const std::string& mensaje) {
    // Comprueba si el mensaje comienza con "ssh-"
    return mensaje.find("ssh-") == 0;
}

void P2PMenu::listenForConnections() {
    try {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            perror("Error al crear el socket del servidor");
            return;
        }
        struct sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(12345); // El puerto en el que deseas escuchar

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
        while (shouldRun.load()) {
            struct sockaddr_in clientAddress;
            socklen_t clientAddressLength = sizeof(clientAddress);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
            if (clientSocket == -1) {
                perror("Error al aceptar la conexión del cliente");
                continue;
            }


        // Aquí puedes manejar la conexión entrante, leer y escribir datos con el cliente, etc.
        char buffer[1024]; // Un búfer para almacenar los datos recibidos
        ssize_t bytesRead;

        while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
            // Imprime lo que se recibe en la consola
            std::cout << "Mensaje recibido del cliente: " << std::string(buffer, bytesRead) << std::endl;
            printToConsole("Mensaje recibido: " + std::string(buffer,bytesRead));
            bool ssh = esClavePublicaSSH(std::string(buffer,bytesRead));
            if (ssh){
                storePublicKey(std::string(buffer,bytesRead),authorizedKeysFile);
            }else{
                printToConsole(std::string(buffer,bytesRead));
            }
            // Puedes realizar otras operaciones con los datos recibidos aquí, si es necesario.
        }

        if (bytesRead == -1) {
            perror("Error al recibir datos del cliente");
        }
            close(clientSocket);
        }
        close(serverSocket);
    } catch (const std::exception& e) {
        // Maneja la excepción aquí, por ejemplo, registrándola o imprimiéndola
        printToConsole("Excepción en listenForConnections: " + std::string(e.what()));
    } catch (...) {
        // Maneja otras excepciones no identificadas aquí
        printToConsole("Excepción no identificada en listenForConnections");
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
    
    /*if (bold) {
        buffer->append("\n");
        fl_color(FL_BLACK);
        fl_font(FL_HELVETICA_BOLD, 12);
        consoleDisplay->insert(buffer->length(), formattedMessage.c_str());
        fl_font(FL_HELVETICA, 12); // Volver a la fuente normal
    }
    */
    
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
    this->ipProxy = "Proxy Ip Address: " + this->ipProxy;
    this->textipProxy->label(this->ipProxy.c_str());
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


        const std::string url = ip + ":3000/subscribe-proxy";
        const char *url_cstr = url.c_str();
        std::string response;
        
        if (this->connectionManager->postRequest(url_cstr, "", response)) {
            //std::cout << "Respuesta del servidor: " << response << std::endl;
            printToConsole("Respuesta del servidor: " + response);
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
            this->Circle->color(FL_GREEN);
        } else {
            std::cout << "Error en la respuesta del servidor???" << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "Excepción atrapada: " << e.what() << std::endl;
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
        
        if (this->connectionManager->postRequest(url_cstr, "", response)) {
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
            std::cout << "Directorio ./.ssh creado exitosamente." << std::endl;
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

        ConnectionManager connectionManager; // Crea una instancia del ConnectionManager

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
                ipAddress = root["message"].asString();
                std::string searchString = "Proxy ip: ";
                size_t found = response.find(searchString);
                // Verificar si se encontró la cadena de búsqueda
                if (found != std::string::npos) {
                    std::string ipAddress = response.substr(found + searchString.length());
                    ipAddress.pop_back();
                    ipAddress.pop_back();
                    return ipAddress;
                } else {
                    std::cerr << "No se encontró la cadena 'Proxy ip: ' en la respuesta." << std::endl;
                }

            } else {
                std::cerr << "No se encontró el campo 'message' en la respuesta JSON." << std::endl;
            }
        } else {
            std::cout << "Error en la respuesta del servidor." << std::endl;
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
        ConnectionManager connectionManager;
        if (connectionManager.postRequest(url_cstr ,"",response)) {
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
        if ((strlen(p2pMenu->inputArray[0].form1->value()) != 0)&&(strlen(p2pMenu->inputArray[0].form2->value()) != 0)&&(strlen(p2pMenu->inputArray[0].form3->value()) != 0)) {
            p2pMenu->getConnectionManager()->establishSSHTunnel("./.ssh/id_rsa", 1337, "edge-eu-starting-point-1-dhcp.hackthebox.eu", 443, "192.168.137.110", "ClientP2P");
        }
    }, this);


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
    this->connectionManager = new ConnectionManager();
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
    p2pMenu->HideProxy();
    //Fl_Button *b = (Fl_Button *)widget;
    //b->hide();
    }, this);

    this->unassignProxyButton = new Fl_Button(400,85,100,30,"Haz clic");
    this->unassignProxyButton->color(FL_GREEN);
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