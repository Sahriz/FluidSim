#include "app.h"

App::App(int width, int height, const char* title)
    : m_width(width), m_height(height) {
    (void)title;
    // TODO: create the GLFW window, load GL, init the renderer.
}

App::~App() {
    // TODO: tear down GL resources and the window.
}

void App::run() {
    // TODO: main loop (poll events, update simulation, render, swap buffers).
}