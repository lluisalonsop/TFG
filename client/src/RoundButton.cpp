#include "RoundButton.h"
#include <FL/fl_draw.H>

RoundButton::RoundButton(int x, int y, int w, int h, const char *label) : Fl_Button(x, y, w, h, label) {
    // Constructor de la clase, puedes realizar cualquier inicialización adicional aquí si es necesario.
}

void RoundButton::draw() {
    // Dibuja un círculo de fondo
    fl_color(FL_GREEN);  // Establece el color de fondo (rojo en este ejemplo)
    int centerX = x() + w() / 2;
    int centerY = y() + h() / 2;
    int radio = 10;  // Ajusta el radio del círculo aquí

    fl_pie(centerX - radio, centerY - radio, 2 * radio, 2 * radio, 0, 360);

    // Calcula la longitud de la línea en función del radio del círculo
    int lineaLength = radio * 0.6;  // Ajusta la longitud de la línea aquí

    // Dibuja una línea vertical
    fl_color(FL_BLACK);  // Establece el color del signo "+" (negro en este ejemplo)
    fl_line(centerX, centerY - lineaLength, centerX, centerY + lineaLength);

    // Dibuja una línea horizontal
    fl_line(centerX - lineaLength, centerY, centerX + lineaLength, centerY);
}