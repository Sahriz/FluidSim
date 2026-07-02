#pragma once

#include <iostream>
#include "Scene.h"
#include <vector>

class TerrainScene : public Scene {
public:
	TerrainScene(int width, int height, std::string title) : m_width(width), m_height(height), Scene(title) {}

	~TerrainScene() override = default;

	void onEnter() override {
		std::string vertPath = std::string(SHADER_DIR) + "render.vert";
		std::string fragPath = std::string(SHADER_DIR) + "render.frag";

		m_shader.init(vertPath, fragPath);

		initTerrain();
		glUseProgram(m_shader.ID);
		float aspectRatio = float(m_width) / float(m_height);
		m_camera.Init(70, aspectRatio, 0.1, 100, m_shader.ID);
		scaleLoc = glGetUniformLocation(m_shader.ID, "scale");
	}

	void initTerrain() {


		glGenBuffers(1, &VBO);

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, std::size(verts) * sizeof(glm::vec3), verts.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	void onExit() override {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

	void update(float dt, frameInput& in) override {
		m_camera.Update(in);
		glUniform1f(scaleLoc, m_scale);
		render();
	}

	void render() override{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(m_shader.ID);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, std::size(indices), GL_UNSIGNED_INT, (void*)0);
	}

	void drawUI() override {
		const char* title = Scene::m_title.c_str();
		ImGui::Begin(title);
		ImGui::SliderFloat("Scale", & m_scale, 0.5f, 2.0f);
		ImGui::End();

	}

private:
	float m_scale = 1.0f;
	Shader m_shader;
	Camera m_camera;
	unsigned int VAO, VBO, EBO;
	int m_width, m_height;
	GLint scaleLoc;
	

	std::vector<glm::vec3> verts = {
		glm::vec3(-0.5,-0.5,-0.5),
		glm::vec3(-0.5,-0.5,0.5),
		glm::vec3(0.5,-0.5,0.5),
		glm::vec3(0.5,-0.5,-0.5),
		glm::vec3(-0.5,0.5,-0.5),
		glm::vec3(-0.5,0.5,0.5),
		glm::vec3(0.5,0.5,0.5),
		glm::vec3(0.5,0.5,-0.5)
	};



	unsigned int indices[36] = {
		//bottom
		0,2,1,
		0,3,2,
		//left
		0,1,4,
		4,1,5,
		//front
		1,2,5,
		5,2,6,
		//right
		3,7,6,
		3,6,2,
		//back
		0,4,3,
		4,7,3,
		//top
		4,5,6,
		7,4,6
	};
};