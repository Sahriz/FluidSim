#pragma once

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "keyHandler.h"

class TerrainCamera {
public:
	TerrainCamera() = default;

	void Init(float fov, float aspectRatio, float nearPlane, float farPlane, GLuint ID) {
		perspectiveMat = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
		perpectiveLoc = glGetUniformLocation(ID, "projection");
		glUniformMatrix4fv(perpectiveLoc, 1, GL_FALSE, glm::value_ptr(perspectiveMat));

		viewLoc = glGetUniformLocation(ID, "view");
		computeView();
	}

	void Update(frameInput& fInput) {
		m_yaw += 0.005 * -fInput.dx;
		m_pitch += 0.005 * fInput.dy;

		if(m_pitch > 1.5f) m_pitch = 1.5f;
		if (m_pitch < -1.5f) m_pitch = -1.5f;

		glm::vec3 forward;
		forward.x = cos(m_pitch) * sin(m_yaw);
		forward.y = sin(m_pitch);
		forward.z = cos(m_pitch) * cos(m_yaw);
		forward = glm::normalize(forward);

		glm::vec3 worldUp = glm::vec3(0.0, 1.0, 0.0);
		glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
		glm::vec3 up = glm::normalize(glm::cross(right, forward));

		float moveSpeed = 0.5f;

		eye += right * fInput.inputDirection.x * moveSpeed;
		eye += up * fInput.inputDirection.y * moveSpeed;
		eye += forward * fInput.inputDirection.z * moveSpeed;

		viewMat = glm::lookAt(eye, eye + forward, worldUp);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMat));
		
	}

	void computeView() {
		

		glm::vec3 forward(cos(m_pitch) * sin(m_yaw), sin(m_pitch), cos(m_pitch) * cos(m_yaw));

		viewMat = glm::lookAt(eye, eye + glm::normalize(forward), glm::vec3(0, 1, 0));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMat));
	}

	glm::mat4 perspectiveMat;
	glm::mat4 viewMat;

	GLint perpectiveLoc;
	GLint viewLoc;
private:
	glm::vec3 eye = glm::vec3(0.0f,25.0f, -100.0f);
	float m_yaw = 0.0f;
	float m_pitch = 0.0f;
	float m_radius = 2.0f;
	glm::vec3 m_target = glm::vec3(0.0f);
};