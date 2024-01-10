#include "Database.h"
#include "WindowManager.h"


int main() {
    Database database;
    WindowManager windowManager;
    Database();

    while(windowManager.isWindowOpen()){
        windowManager.handleEvents(database);
        windowManager.renderWindow();

    }

}

