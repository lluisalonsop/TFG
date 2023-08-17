#ifndef P2P_MENU_H
#define P2P_MENU_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include "ConnectionManager.h"

class P2PMenu {
public:
    P2PMenu();
    void run();
    void buttonCallback(Fl_Widget *widget, void *data);

private:
    Fl_Window *window;
    Fl_Button *button;
    Fl_Box *text;
    ConnectionManager *connectionManager;
};

#endif // P2P_MENU_H