#include "RoundButton.h"
#include <FL/fl_draw.H>

RoundButton::RoundButton(int x, int y, int w, int h, const char *symbol) : Fl_Button(x, y, w, h), symbol_(symbol) {
    // Constructor de la clase, puedes realizar cualquier inicialización adicional aquí si es necesario.
}

void RoundButton::draw() {
    // Dibuja un círculo de fondo
    if (std::strcmp(symbol_, "-") == 0) {
        fl_color(FL_RED);
    } else {
        fl_color(FL_GREEN);
    }  
    int centerX = x() + w() / 2;
    int centerY = y() + h() / 2;
    int radio = 10;  // Ajusta el radio del círculo aquí

    fl_pie(centerX - radio, centerY - radio, 2 * radio, 2 * radio, 0, 360);

    // Calcula la longitud de la línea en función del radio del círculo
    int lineaLength = radio * 0.6;  // Ajusta la longitud de la línea aquí

    // Dibuja el símbolo en lugar de una cruz
    fl_color(FL_BLACK);  // Establece el color del símbolo (negro en este ejemplo)
    
    // Dibuja el símbolo personalizado en función del valor de "symbol_"
    if (std::strcmp(symbol_, "-") == 0) {
        // Dibuja una línea horizontal
        fl_line(centerX - lineaLength, centerY, centerX + lineaLength, centerY);
    } else {
        // Dibuja el símbolo predeterminado (cruz)
        fl_line(centerX, centerY - lineaLength, centerX, centerY + lineaLength);
        fl_line(centerX - lineaLength, centerY, centerX + lineaLength, centerY);
    }
}