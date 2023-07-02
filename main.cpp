#include "SDLApp.hpp"

int main(int , char* []) {
    try {
        SDLApp app;
        app.exec();
    }
    catch(const SDLAppException& e) {
        return e.errcode();
    }
    return 0;
}
