#ifndef P2P_MENU_H
#define P2P_MENU_H


#include "ConnectionManager.h"
#include "RoundButton.h"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Input.H>
#include <fstream>
#include <ctime>
#include <json/json.h>
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <atomic>

struct InputGroup {
    Fl_Input *form1;
    Fl_Input *form2;
    Fl_Input *form3;
    Fl_Button *establishTunnel;
    Fl_Button *disconnect;
};

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
    std::string askForProxy(Fl_Widget *widget, void *data);
    void HideOffer();
    void ShowOffer();
    bool areKeysPresent();
    void unassignProxy(Fl_Widget *widget, void *data);
    void listenForConnections();
    void HideProxy();
    void ShowProxy();
    void initRoundButton(int x, int y, int w, int h, const char *label);
    void drawInputs();
    void addConnection();
    void substractConnection();
    int getNumConnections();
    ConnectionManager* getConnectionManager();
private:
    std::string ipProxy;
    InputGroup inputArray[4];
    Fl_Window *window;
    Fl_Button *unassignProxyButton;
    Fl_Button *buttonOffer;
    Fl_Button *unsubscribeOffer;
    Fl_Button *clientButton;
    Fl_Box *textipProxy;
    Fl_Box *sessionInfo;
    Fl_Box *assignProxy;
    Fl_Box *unassignProxytext;
    Fl_Box *proxyUnsubscribeText;
    Fl_Box *proxyOfferText;
    Fl_Box *consoleText;
    Fl_Box *servingStatus;
    ConnectionManager *connectionManager;
    Fl_Text_Buffer *consoleBuffer;
    Fl_Text_Display *consoleDisplay;
    Fl_Button *clearButton;
    Fl_Box *Circle;
    RoundButton *roundButton;
    RoundButton *roundButtonSubstract;
    int numConnections;
};

#endif // P2P_MENU_H