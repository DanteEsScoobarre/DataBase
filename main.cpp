#include "Database.h"
#include "WindowManager.h"


int main() {
    Database database;
    Database();


    WindowManager windowManager;
    while(windowManager.isWindowOpen()){
        windowManager.handleEvents(database);
        windowManager.renderWindow();

    }

}

