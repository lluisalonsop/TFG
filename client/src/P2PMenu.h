#ifndef P2P_MENU_H
#define P2P_MENU_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>

class P2PMenu {
public:
    P2PMenu();
    void run();

private:
    Fl_Window *window;
    Fl_Button *button;
};

#endif // P2P_MENU_H