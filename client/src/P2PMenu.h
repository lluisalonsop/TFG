#ifndef P2P_MENU_H
#define P2P_MENU_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <fstream>
#include "ConnectionManager.h"
#include <FL/Fl_Text_Display.H>

class P2PMenu {
public:
    P2PMenu();
    void run();
    void buttonCallback(Fl_Widget *widget, void *data);
    void printToConsole(const std::string &message);
    void clearButtonCallback();
    void HideOffer();

private:
    Fl_Window *window;
    Fl_Button *button;
    Fl_Box *proxyOfferText;
    Fl_Box *consoleText;
    ConnectionManager *connectionManager;
    Fl_Text_Buffer *consoleBuffer;
    Fl_Text_Display *consoleDisplay;
    Fl_Button *clearButton;
    Fl_Box *greenCircle;
};

#endif // P2P_MENU_H