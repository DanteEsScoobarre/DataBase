#include "WindowManager.h"
#include "PreRequistion.h"


WindowManager::WindowManager() : window(sf::VideoMode(1200, 400), "Database") {
    if (!font.loadFromFile("..\\Font\\Montserrat-VariableFont_wght.ttf")) {
        std::cerr << "Failed to load font." << std::endl;
    }
    cursorRect.setSize(sf::Vector2f(0.8f, 20.f));
    cursorRect.setPosition(11, 32);
    cursorRect.setFillColor(sf::Color::White);

    welcomeText.setFont(font);
    welcomeText.setCharacterSize(20);
    welcomeText.setPosition(10, 10);
    welcomeText.setString("Witaj, co chcesz dzisiaj zrobic");

    inputText.setFont(font);
    inputText.setCharacterSize(20);
    inputText.setPosition(10, 30);

    outputText.setFont(font);
    outputText.setCharacterSize(20);
    outputText.setPosition(10, 60);

    errorText.setFont(font);
    errorText.setCharacterSize(20);
    errorText.setPosition(10, 90);
    errorText.setFillColor(sf::Color::Red);
}

void WindowManager::handleEvents(Database& myDatabase) {
    sf::Event event;
    float cursorPositionX = inputText.findCharacterPos(inputText.getString().getSize()).x;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }

        if (event.type == sf::Event::TextEntered) {


            if (event.text.unicode == 13) { // Enter key
                std::string userInput = inputText.getString();
                if (userInput == "exit") {
                    window.close();
                } else {
                    try {
                        if (currentOperation.empty()) {
                            cursorRect.setPosition(cursorPositionX, 32);
                            std::streambuf *coutbuf = std::cout.rdbuf();
                            std::cout.rdbuf(oss.rdbuf());

                            myDatabase.executeQuery(userInput);

                            std::cout.rdbuf(coutbuf);

                            outputText.setString(oss.str());
                            oss.str("");
                        } else if (currentOperation == "create table") {
                            // Przykładowa obsługa tworzenia tabeli z nazwą podaną przez użytkownika
                            myDatabase.createTable(currentTableName);

                            outputText.setString("Table " + currentTableName + " created successfully.");
                        }
                        // Dodaj obsługę innych operacji, takich jak "insert into", "update", itp., w podobny sposób
                    } catch (const std::exception& e) {
                        showError("Error: " + std::string(e.what()));
                    }

                    // Zakończ bieżącą operację
                    currentOperation.clear();
                    currentTableName.clear();
                }

                inputText.setString("");
            } else if (event.text.unicode == 8) { // Backspace key
                std::string currentText = inputText.getString();
                if (!currentText.empty()) {
                    currentText.pop_back();
                    inputText.setString(currentText);

                    cursorRect.setPosition(cursorPositionX, 32);
                }
            } else {
                std::string currentText = inputText.getString();
                currentText += static_cast<char>(event.text.unicode);
                inputText.setString(currentText);

                // Aktualizuj pozycję kursora
                float cursorPositionX = inputText.findCharacterPos(inputText.getString().getSize()).x;
                cursorRect.setPosition(cursorPositionX, 32);

                // Sprawdź, czy użytkownik wprowadza nazwę operacji (np. "create table")
                if (currentText.find("create table") != std::string::npos) {
                    currentOperation = "create table";
                } else if (currentText.find("select table") != std::string::npos) {
                    // Przykład obsługi operacji "select table" - pozyskaj nazwę tabeli
                    size_t startPos = currentText.find("select table") + std::strlen("select table") + 1;
                    currentTableName = currentText.substr(startPos, currentText.find(" ", startPos) - startPos);
                }
                // Dodaj obsługę innych operacji, jeśli potrzebujesz
            }
        }
    }
}

void WindowManager::renderWindow() {
    window.clear();
    window.draw(welcomeText);
    window.draw(cursorRect);
    window.draw(inputText);
    window.draw(outputText);
    window.draw(errorText);
    window.display();
}

bool WindowManager::isWindowOpen() const {
    return window.isOpen();
}

void WindowManager::showError(const std::string &errorMessage) {
    errorText.setString(errorMessage);
}



