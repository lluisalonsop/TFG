#pragma once
#include <iostream>
#include <cstring>
#include <FL/Fl_Button.H>

class RoundButton : public Fl_Button {
public:
    RoundButton(int x, int y, int w, int h,const char *symbol);
    void draw() override;
private:
    const char* symbol_;
};