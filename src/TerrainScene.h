#pragma once

#include <iostream>
#include "Scene.h"
#include <vector>


class TerrainScene : public Scene {
public:
	TerrainScene(int width, int height, std::string title) :Scene(title), m_width(width), m_height(height) {
		for (size_t x = 0; x < meshWidth; x++) {
			for (size_t z = 0; z < meshDepth; z++) {
				glm::vec3 position = glm::vec3(x - meshWidth / 2.0, -1.0, z-meshDepth/2.0);
				verts.push_back(position);
			}
		}
		for (size_t x = 0; x < meshWidth - 1; x++) {
			for (size_t z = 0; z < meshDepth - 1; z++) {
				int id1 = z + x * meshWidth;
				int id2 = z + (x + 1) * meshWidth;
				int id3 = id1 + 1;
				int id4 = id2 + 1;
				indices.push_back(id1);
				indices.push_back(id3);
				indices.push_back(id2);
				indices.push_back(id4);
				indices.push_back(id2);
				indices.push_back(id3);
			}
		}
	}

	~TerrainScene() override = default;

	void onEnter() override {
		std::string vertPath = std::string(SHADER_DIR) + "TerrainRender.vert";
		std::string fragPath = std::string(SHADER_DIR) + "TerrainRender.frag";

		m_shader.init(vertPath, fragPath);

		initTerrain();
		glUseProgram(m_shader.ID);
		float aspectRatio = float(m_width) / float(m_height);
		m_camera.Init(70, aspectRatio, 0.1, 1000, m_shader.ID);
		scaleLoc = glGetUniformLocation(m_shader.ID, "scale");
		noiseScaleLoc = glGetUniformLocation(m_shader.ID, "noiseScale");
		ampLoc = glGetUniformLocation(m_shader.ID, "amplitude");
		sampleScaleLoc = glGetUniformLocation(m_shader.ID, "sampleScale");
		freqLoc = glGetUniformLocation(m_shader.ID, "frequency");
		lacLoc = glGetUniformLocation(m_shader.ID, "lacunarity");
		perLoc = glGetUniformLocation(m_shader.ID, "persistance");
	}

	void initTerrain() {


		glGenBuffers(1, &VBO);

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, std::size(indices)*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

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
		glUniform1f(noiseScaleLoc, m_noiseScale);
		glUniform1f(ampLoc, m_noiseAmp);
		glUniform1f(sampleScaleLoc, m_sampleScale);
		glUniform1f(freqLoc, m_frequency);
		glUniform1f(lacLoc, m_lacunarity);
		glUniform1f(perLoc, m_persistance);
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
		ImGui::SliderFloat("Scale", & m_scale, 0.05f, 1.0f);
		ImGui::SliderFloat("noiseScale", &m_noiseScale, 0.005f, 0.08f);
		ImGui::SliderFloat("sampleScale", &m_sampleScale, 0.005f, 0.08f);
		ImGui::SliderFloat("Amplitude", &m_noiseAmp, 0.1f, 200.0f);
		ImGui::SliderFloat("Frequency", &m_frequency, 20.0f, 2000.0f);
		ImGui::SliderFloat("Lacunarity", &m_lacunarity, 1.0f, 8.0f);
		ImGui::SliderFloat("Persistance", &m_persistance, 0.1f, 1.0f);
		ImGui::End();

	}

private:
	float m_scale = 0.1f, m_noiseScale = 0.02, m_sampleScale = 0.01f, m_noiseAmp = 50.0, m_frequency = 200.0f, m_lacunarity = 2.0f, m_persistance = 0.5f;
	int meshWidth = 1000, meshDepth = 1000;
	Shader m_shader;
	TerrainCamera m_camera;
	unsigned int VAO, VBO, EBO;
	int m_width, m_height;
	GLint scaleLoc, noiseScaleLoc, sampleScaleLoc, ampLoc, freqLoc, lacLoc, perLoc;

	std::vector<glm::vec3> verts;
	std::vector<unsigned int> indices;
};