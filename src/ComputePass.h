#pragma once

#include "Shader.h"
#include "glm.hpp"
#include <functional>
#include <string>
#include <vector>

struct ComputePass {
	std::string name;
	Shader shader;
	glm::ivec3 groups{ 1,1,1 };
	int iterations = 1;
	GLbitfield barrierAfter = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
	bool enabled = true;
	bool continuous = true;
	bool dirty = true;
	std::function<void(Shader&, int)> bind;
};

inline void runPipeline(std::vector<ComputePass>& passes) {
	for (auto& p : passes) {
		if(!p.enabled || (!p.continuous && !p.dirty)) continue;
		p.shader.use();
		for (int i = 0; i < p.iterations; i++) {
			if (p.bind) p.bind(p.shader, i);
			glDispatchCompute(p.groups.x, p.groups.y, p.groups.z);
			glMemoryBarrier(p.barrierAfter);
		}
		p.dirty = false;
	}
}
