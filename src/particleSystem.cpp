#include "particleSystem.h"

ParticleSystem::ParticleSystem() {}
ParticleSystem::~ParticleSystem() {}

void ParticleSystem::init(int particleCount) {
    m_particleCount = particleCount;
    // TODO: allocate buffers / seed initial state.
}

void ParticleSystem::update(float deltaTime) {
    (void)deltaTime;
    // TODO: advance the simulation.
}