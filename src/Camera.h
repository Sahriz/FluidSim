#pragma once

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "keyHandler.h"

class Camera {
public:
	Camera() = default;

	void Init(float fov, float aspectRatio, float nearPlane, float farPlane, GLuint ID) {
		perspectiveMat = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
		perpectiveLoc = glGetUniformLocation(ID, "projection");
		glUniformMatrix4fv(perpectiveLoc, 1, GL_FALSE, glm::value_ptr(perspectiveMat));

		viewLoc = glGetUniformLocation(ID, "view");
		recomputeView();
	}

	void Update(frameInput& fInput) {
		m_yaw += 0.005 * fInput.dx;
		m_pitch += 0.005 * fInput.dy;
		//std::cout << "dx: " << fInput.dx << " | dy: " << fInput.dy << " | yaw: " << m_yaw << " | pitch: " << m_pitch << std::endl;
		recomputeView();
	}

	void recomputeView() {
		glm::vec3 eye;

		eye.x = m_target.x + m_radius * cos(m_pitch) * sin(m_yaw);
		eye.y = m_target.y + m_radius * sin(m_pitch);
		eye.z = m_target.z + m_radius * cos(m_pitch) * cos(m_yaw);

		viewMat = glm::lookAt(eye, m_target, glm::vec3(0, 1, 0));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMat));
	}

	glm::mat4 perspectiveMat;
	glm::mat4 viewMat;

	GLint perpectiveLoc;
	GLint viewLoc;
private:
	float m_yaw = 0.0f;
	float m_pitch = 0.0f;
	float m_radius = 2.0f;
	glm::vec3 m_target = glm::vec3(0.0f);
};