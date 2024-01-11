#ifndef DATABASE_WINDOWMANAGER_H
#define DATABASE_WINDOWMANAGER_H
#include "PreRequistion.h"
#include "Database.h"
class WindowManager {
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Text inputText;
    sf::Text outputText;
    std::ostringstream oss;
    sf::Text errorText;
    std::string currentOperation;
    sf::Text welcomeText;
    std::string currentTableName;
    sf::RectangleShape cursorRect;
public:
    WindowManager();
    void handleEvents(Database& myDatabase);
    void renderWindow();
    bool isWindowOpen() const;
    //Errors
    void showError(const std::string& errorMessage);

};




#endif //DATABASE_WINDOWMANAGER_H
