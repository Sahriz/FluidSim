#include "app.h"
#include <iostream>

int main() {
    try {
        App app(1280, 720, "Application");
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}