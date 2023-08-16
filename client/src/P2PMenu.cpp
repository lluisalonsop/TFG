#include "P2PMenu.h"

P2PMenu::P2PMenu() {
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

    // Crear un botón en la esquina superior izquierda
    this->button = new Fl_Button(10, 10, 100, 30, "Haz clic");
    Fl_Color green = 79;

    this->button->color(green);

    this->button->callback([](Fl_Widget *widget, void *data) {
        Fl_Button *b = (Fl_Button *)widget;
        b->label("¡Hiciste clic!");
    });
    this->window->end();
}

void P2PMenu::run() {
    this->window->show();
    Fl::run();
}