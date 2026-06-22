#pragma once

// Owns simulation state and the data buffers the renderer reads.
class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

    void init(int particleCount);
    void update(float deltaTime);

private:
    int m_particleCount = 0;
};