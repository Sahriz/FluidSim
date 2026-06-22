#include "renderer.h"

Renderer::Renderer() {}
Renderer::~Renderer() {}

bool Renderer::init(int width, int height) {
    (void)width; (void)height;
    // TODO: compile shaders, create buffers/VAOs.
    return true;
}

void Renderer::render(ParticleSystem& system) {
    (void)system;
    // TODO: issue draw calls.
}

void Renderer::cleanup() {
    // TODO: release GL resources.
}