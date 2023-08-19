#include "P2PMenu.h"
const Fl_Color green = 79;

bool containsSubstring(const std::string &line, const std::string &substring) {
    return line.find(substring) != std::string::npos;
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
        const char *url = "http://localhost:3000/subscribe-proxy";
        std::string response;
        
        if (this->connectionManager->postRequest(url, "", response)) {
            std::cout << "Respuesta del servidor: " << response << std::endl;
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
    this->text = new Fl_Box(50, 10, 100, 20, "Ofrecerse como proxy");

    // Configurar el color y estilo del widget de texto
    this->text->box(FL_NO_BOX); // Eliminar el borde del widget de texto

    // Cambiar el color de fondo del widget de texto a blanco (RGB: 255, 255, 255)
    this->text->color(FL_WHITE);

    // Crear un botón en la esquina superior izquierda
    this->button = new Fl_Button(50, 50, 100, 30, "Haz clic");

    this->button->color(green);

    this->button->callback([](Fl_Widget *widget, void *data) {
    P2PMenu *p2pMenu = static_cast<P2PMenu *>(data);
    p2pMenu->buttonCallback(widget, p2pMenu);
    Fl_Button *b = (Fl_Button *)widget;
    b->label("¡Hiciste clic!");
    }, this);

    this->window->end();
}

void P2PMenu::run() {
    this->window->show();
    Fl::run();
}