#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

int main() {
    Fl_Window win(300, 150, "Test");
    Fl_Box box(20, 40, 260, 30, "FLTK works!");
    win.end();
    win.show();
    return Fl::run();
}