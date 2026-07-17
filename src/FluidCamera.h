#pragma once

#include <glad/glad.h>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "keyHandler.h"

struct CameraBlock {
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 viewProj;
	glm::mat4 invViewProjMat;
	glm::vec4 camPos;
};

class FluidCamera {
public:
	FluidCamera() = default;
	~FluidCamera() { if (m_ubo) glDeleteBuffers(1, &m_ubo); }
	FluidCamera(const FluidCamera&) = delete;
	FluidCamera& operator=(const FluidCamera&) = delete;

	void Init(float fov, float aspectRatio, float nearPlane, float farPlane) {
		m_fov = fov; m_near = nearPlane; m_far = farPlane;
		m_proj = glm::perspective(glm::radians(m_fov), aspectRatio, m_near, m_far);

		glCreateBuffers(1, &m_ubo);
		glNamedBufferStorage(m_ubo, sizeof(CameraBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_ubo);

		upload();
	}

	void Update(float dt, frameInput& in) {
		m_yaw += 0.005 * float(-in.dx);
		m_pitch += 0.005 * float(in.dy);
		m_pitch = glm::clamp(m_pitch, -1.5f, 1.5f);

		glm::vec3 forward;
		forward.x = cos(m_pitch) * sin(m_yaw);
		forward.y = sin(m_pitch);
		forward.z = cos(m_pitch) * cos(m_yaw);
		forward = glm::normalize(forward);

		glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
		glm::vec3 up = glm::normalize(glm::cross(right, forward));

		m_eye += (right * in.inputDirection.x
				+ up * in.inputDirection.y
				+ forward * in.inputDirection.z) * m_moveSpeed * dt;

		m_view = glm::lookAt(m_eye, m_eye + forward, glm::vec3(0,1,0));
		upload();
	}

	void setAspect(float aspect) {
		m_proj = glm::perspective(glm::radians(m_fov), aspect, m_near, m_far);
		upload();
	}

private:
	void upload() {
		CameraBlock b;
		b.view = m_view;
		b.projection = m_proj;
		b.viewProj = m_proj * m_view;
		b.invViewProjMat = glm::inverse(b.viewProj);
		b.camPos = glm::vec4(m_eye, 1.0f);
		glNamedBufferSubData(m_ubo, 0, sizeof(b), &b);
	}
	
	GLuint m_ubo = 0;
	glm::mat4 m_view{ 1.0f }, m_proj{ 1.0f };
	glm::vec3 m_eye{ 0.0f, 25.0f, -100.0f };
	float m_yaw = 0.0f, m_pitch = 0.0f;
	float m_fov = 70.0f, m_near = 0.1f, m_far = 1000.0f;
	float m_moveSpeed = 300.0f;
};