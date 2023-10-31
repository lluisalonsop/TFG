#pragma once

#include <FL/Fl_Button.H>

class RoundButton : public Fl_Button {
public:
    RoundButton(int x, int y, int w, int h, const char *label = 0);
    void draw() override;
};