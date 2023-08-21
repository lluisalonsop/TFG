#include "P2PMenu.h"
#include <ctime>
const Fl_Color green = 79;

bool containsSubstring(const std::string &line, const std::string &substring) {
    return line.find(substring) != std::string::npos;
}

/*void P2PMenu::printToConsole(const std::string &message) {
    this->consoleBuffer->append(message.c_str());
    this->consoleBuffer->append("\n");
}
*/

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
    this->button->hide();
    this->proxyOfferText->hide();
    this->greenCircle->show();
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

bool isPublicKeyPresent(const std::string &substringToCheck, const std::string &authorizedKeysFile) {
    std::ifstream file(authorizedKeysFile);
    if (!file) {
        std::cerr << "Error al abrir el archivo: " << authorizedKeysFile << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (containsSubstring(line, substringToCheck)) {
            return true; // La clave pública está presente
        }
    }

    return false; // La clave pública no se encontró en authorized_keys
}

void P2PMenu::buttonCallback(Fl_Widget *widget, void *data) {
    std::cout << "Aquí llegamos 3" << std::endl;
    
    try {

        std::string substringToCheck = "P2PServerKey"; // Subcadena a buscar
        std::string authorizedKeysFile = "/home/ClientP2P/.ssh/authorized_keys";
        isPublicKeyPresent(substringToCheck,authorizedKeysFile);
        const char *url = "http://192.168.137.38:3000/subscribe-proxy";
        std::string response;
        
        if (this->connectionManager->postRequest(url, "", response)) {
            //std::cout << "Respuesta del servidor: " << response << std::endl;
            printToConsole("Respuesta del servidor: " + response);
        } else {
            std::cout << "Error en la respuesta del servidor???" << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "Excepción atrapada: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Excepción no identificada atrapada" << std::endl;
    }
}

P2PMenu::P2PMenu() {
    this->connectionManager = new ConnectionManager();
    std::cout << "Aquí llegamos 1 " << std::endl;
    // Obtener el ancho y alto de la pantalla
    int screenWidth = Fl::w();
    int screenHeight = Fl::h();

    std::cout << "screenWidth:" <<  screenWidth <<std::endl;
    std::cout << "screenHeight:" <<  screenWidth <<std::endl;

    // Calcular las dimensiones de la ventana
    int windowWidth = screenWidth * 0.8; // Por ejemplo, el 80% del ancho
    int windowHeight = screenHeight * 0.8; // Por ejemplo, el 80% del alto

    // Crear una ventana centrada en la pantalla
    int windowX = (screenWidth - windowWidth) / 2;
    int windowY = (screenHeight - windowHeight) / 2;
    this->window = new Fl_Window(windowWidth, windowHeight, "P2P Menu");
    this->window->position(windowX, windowY);

    // Cambiar el color de fondo de la ventana a un azul claro (RGB: 173, 216, 230)
    Fl::set_color(FL_BACKGROUND_COLOR, 173, 216, 230);
    window->color(FL_BACKGROUND_COLOR);

    //Crear texto encima del botón
    this->proxyOfferText = new Fl_Box(50, 10, 100, 20, "Ofrecerse como proxy");

    // Configurar el color y estilo del widget de texto
    this->proxyOfferText->box(FL_NO_BOX); // Eliminar el borde del widget de texto

    // Cambiar el color de fondo del widget de texto a blanco (RGB: 255, 255, 255)
    this->proxyOfferText->color(FL_WHITE);

    this->consoleText = new Fl_Box(675,385,100,20, "Console logs");
    this->consoleText->box(FL_NO_BOX);
    this->consoleText->color(FL_WHITE);

    // Crear un botón en la esquina superior izquierda
    this->button = new Fl_Button(50, 50, 100, 30, "Haz clic");

    this->button->color(green);

    this->button->callback([](Fl_Widget *widget, void *data) {
    P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
    p2pMenu->buttonCallback(widget, p2pMenu);
    p2pMenu->HideOffer();
    //Fl_Button *b = (Fl_Button *)widget;
    //b->hide();
    }, this);

    this->greenCircle = new Fl_Box(1400, 50, 25, 25); // Tamaño del círculo
    this->greenCircle->box(FL_ROUND_DOWN_BOX); // Establece el estilo redondeado
    this->greenCircle->color(FL_GREEN); // Establece el color
    this->greenCircle->hide(); // Oculta el círculo al principio

    this->clearButton = new Fl_Button(1300, 380, 100, 30, "Clear");
    this->clearButton->color(green);
    this->clearButton->callback(static_clearButtonCallback, this);

    this->consoleBuffer = new Fl_Text_Buffer();
    this->consoleDisplay = new Fl_Text_Display(50, 420, windowWidth - 100, windowHeight - 450);
    this->consoleDisplay->buffer(consoleBuffer);
    this->consoleDisplay->textfont(FL_COURIER);

    this->window->end();
}

void P2PMenu::run() {
    this->window->show();
    Fl::run();
}