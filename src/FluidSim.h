#pragma once

// Owns simulation state and the data buffers the renderer reads.
class FluidSim {
public:
    FluidSim();
    ~FluidSim();

    void init(int particleCount);
    void update(float deltaTime);

private:
    int dimensionSize = 512;
};