#ifndef P2P_MENU_H
#define P2P_MENU_H


#include "ConnectionManager.h"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Display.H>
#include <fstream>
#include <ctime>
#include <json/json.h>
#include <iostream>
#include <cstdlib>
#include <filesystem>

class P2PMenu {
public:
    P2PMenu();
    void run();
    void buttonOfferCallback(Fl_Widget *widget, void *data);
    void buttonUnsubscribeCallback(Fl_Widget *widget, void *data);
    void printToConsole(const std::string &message);
    bool isPublicKeyPresent(const std::string &substringToCheck, const std::string &authorizedKeysFile);
    bool storePublicKey(const std::string pubKey ,const std::string &authorizedKeysFile);
    void clearButtonCallback();
    void askForProxy(Fl_Widget *widget, void *data);
    void HideOffer();
    void ShowOffer();
    bool areKeysPresent();
    void unassignProxy(Fl_Widget *widget, void *data);
private:
    Fl_Window *window;
    Fl_Button *unassignProxyButton;
    Fl_Button *buttonOffer;
    Fl_Button *unsubscribeOffer;
    Fl_Button *clientButton;
    Fl_Box *assignProxy;
    Fl_Box *proxyUnsubscribeText;
    Fl_Box *proxyOfferText;
    Fl_Box *consoleText;
    Fl_Box *servingStatus;
    ConnectionManager *connectionManager;
    Fl_Text_Buffer *consoleBuffer;
    Fl_Text_Display *consoleDisplay;
    Fl_Button *clearButton;
    Fl_Box *Circle;
};

#endif // P2P_MENU_H