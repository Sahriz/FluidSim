#pragma once

#include <iostream>
#include "Scene.h"
#include <vector>
#include "FluidCamera.h"
#include <fstream>
#include <sstream>
#include "ComputePass.h"

class FluidSim : public Scene {
public:
	FluidSim(int width, int height, std::string title) :Scene(title), m_width(width), m_height(height) {

	}

	~FluidSim() override = default;

	template <typename T>
	void CreateBuffer(GLuint& ID, size_t size) {
		glGenBuffers(1, &ID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(T), NULL, GL_DYNAMIC_COPY);
	}

	void buildPipeline() {
		m_pipeline.clear();

		ComputePass base;
		base.name = "Cloud base";
		base.shader.init(std::string(SHADER_DIR) + "CloudBase.comp");
		base.groups = glm::ivec3(dimensions / 8);
		base.barrierAfter = GL_TEXTURE_FETCH_BARRIER_BIT;
		
		base.bind = [this](Shader& s, int) {
			glBindImageTexture(0, volumeTex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

			s.set("amplitude", amplitude);
			s.set("frequency", frequency);
			s.set("lacunarity", lacunarity);
			s.set("persistance", persistance);
			s.set("octaves", octaves);
			s.set("time", time);
			s.set("dimension", dimensions);
			s.set("numCells", numCells);
			s.set("timeScale", timeScale);
			s.set("useTimeScale", timeEffect ? 1 : 0);
			};

		m_pipeline.push_back(std::move(base));

		ComputePass detail;
		detail.name = "Cloud detail";
		detail.shader.init(std::string(SHADER_DIR) + "CloudDetail.comp");
		detail.groups = glm::ivec3(dimensions / (4 * 8));
		detail.barrierAfter = GL_TEXTURE_FETCH_BARRIER_BIT;
		detail.bind = [this](Shader& s, int) {
			glBindImageTexture(0, detailTex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

			s.set("amplitude", amplitudeDetail);
			s.set("lacunarity", lacunarityDetail);
			s.set("persistance", persistanceDetail);
			s.set("octaves", octavesDetail);
			s.set("time", time);
			s.set("dimension", dimensions / 4);
			s.set("numCells", numCellsDetail);
			s.set("timeScale", timeScaleDetail);
			s.set("useTimeScale", timeEffectDetail ? 1 : 0);
			};
		m_pipeline.push_back(std::move(detail));
	}

	void InitTextures() {
		glGenTextures(1, &volumeTex);
		glBindTexture(GL_TEXTURE_3D, volumeTex);
		glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, dimensions, dimensions, dimensions);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		int detailSize = dimensions / 4;

		glGenTextures(1, &detailTex);
		glBindTexture(GL_TEXTURE_3D, detailTex);
		glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, detailSize, detailSize, detailSize);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

		glGenTextures(1, &frameTex);
		glBindTexture(GL_TEXTURE_2D, frameTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	void onEnter() override {
		std::string vertPath = std::string(SHADER_DIR) + "FluidRender.vert";
		std::string fragPath = std::string(SHADER_DIR) + "FluidRender.frag";

		m_shader.init(vertPath, fragPath);

		InitProgram();
		glUseProgram(m_shader.ID);
		float aspectRatio = float(m_width) / float(m_height);
		m_camera.Init(70, aspectRatio, 0.1, 1000);
		m_shader.set("boxMin", -dimensions / 2);
		m_shader.set("boxMax", dimensions / 2);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   // src*alpha + dst*(1-alpha)
	}

	void InitBuffers() {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		volumeLoc = glGetUniformLocation(m_shader.ID, "volume");
		detailLoc = glGetUniformLocation(m_shader.ID, "detailNoise");
	}

	void InitProgram() {
		InitBuffers();
		InitTextures();
		buildPipeline();
		runPipeline(m_pipeline);
	}

	void onExit() override {
		glDeleteVertexArrays(1, &VAO);
		glDeleteTextures(1, &volumeTex);
		glDeleteTextures(1, &detailTex);
		glDeleteTextures(1, &frameTex);
	}

	void onResize(int width, int height) override {
		m_width = width;
		m_height = height;

		float aspectRatio = float(m_width) / float(m_height);
		m_camera.setAspect(aspectRatio);

		if (frameTex != 0) {
			glBindTexture(GL_TEXTURE_2D, frameTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
		}
	}

	void update(float dt, frameInput& in) override {
		
		m_camera.Update(dt, in);
		time += dt;

		
	}

	void updateUniforms() {
		m_shader.use();
		m_shader.set("detailScale", detailScale);
		m_shader.set("detailStrength", detailStrength);
		m_shader.set("densityScale", densityScale);
		m_shader.set("lightDir", lightDir);
		m_shader.set("lightColor", lightColor);
		m_shader.set("lightStrength", lightStrength);
		m_shader.set("skyColor", skyColor);
		m_shader.set("exposure",exposure);
		m_shader.set("shadowDensity", shadowDensity);
		m_shader.set("phaserG", phaserG);
		m_shader.set("nearShadowReach", nearShadowReach);
		m_shader.set("farShadowReach", farShadowReach);
		m_shader.set("stepSize", stepSize);
		m_shader.set("maxSteps", maxSteps);
		m_shader.set("powderMix", powderMix);
		m_shader.set("ambientColorTop", ambientColorTop);
		m_shader.set("ambientColorBottom", ambientColorBottom);
		m_shader.set("useJitter", useJitter ? 1 : 0);
		m_shader.set("useDetailNoise", useDetailNoise ? 1 : 0);
	}

	void updateLightDirection() {
		float az = glm::radians(azimuth), el = glm::radians(elevation);
		lightDir = glm::vec3(cos(el) * sin(az), sin(el), cos(el) * cos(az));
	}

	void render() override {
		glClearColor(0.2f, 0.3f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		m_pipeline[PassBase].continuous = timeEffect;
		m_pipeline[PassDetail].continuous = timeEffectDetail;
		runPipeline(m_pipeline);
		m_shader.use();
		updateUniforms();
		updateLightDirection();
		glBindVertexArray(VAO);
		glUniform1i(volumeLoc, 0);
		glUniform1i(detailLoc, 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, volumeTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, detailTex);

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	void drawUI() override {
		//First section

		
		if (ImGui::CollapsingHeader("Cloud Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
			auto& basePass = m_pipeline[PassBase];
			if (ImGui::CollapsingHeader("Density Noise Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
				basePass.dirty |= ImGui::SliderFloat("Amplitude", &amplitude, 0.0f, 5.0f);
				basePass.dirty |= ImGui::SliderFloat("Frequency", &frequency, 0.0f, 0.1f);
				basePass.dirty |= ImGui::SliderFloat("Persistance", &persistance, 0.0f, 1.0f);
				basePass.dirty |= ImGui::SliderFloat("Lacunarity", &lacunarity, 0.0f, 5.0f);
				basePass.dirty |= ImGui::SliderInt("Octaves", &octaves, 1, 10);
				basePass.dirty |= ImGui::SliderFloat("Number of Cells", &numCells, 2.0f, 64.0f);
				basePass.dirty |= ImGui::SliderFloat("Time Scale", &timeScale, 0.0f, 25.0f);
				basePass.dirty |= ImGui::Checkbox("Time Effect", &timeEffect);
			}
		
			ImGui::Separator();
		
			//Second section
			auto& detailPass = m_pipeline[PassDetail];
			if (ImGui::CollapsingHeader("Detail Noise Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
				detailPass.dirty |= ImGui::SliderFloat("Detail Amplitude", &amplitudeDetail, 16, 256);
				detailPass.dirty |= ImGui::SliderFloat("Detail Persistance", &persistanceDetail, 0.0f, 1.0f);
				detailPass.dirty |= ImGui::SliderFloat("Detail Lacunarity", &lacunarityDetail, 0.0f, 5.0f);
				detailPass.dirty |= ImGui::SliderInt("Detail Octaves", &octavesDetail, 1, 10);
				detailPass.dirty |= ImGui::SliderFloat("Detail Number of Cells", &numCellsDetail, 2.0f, 64.0f);
				detailPass.dirty |= ImGui::SliderFloat("Detail Time Scale", &timeScaleDetail, 0.0f, 25.0f);
				detailPass.dirty |= ImGui::Checkbox("Detail Time Effect", &timeEffectDetail);
			}
		
			ImGui::Spacing();
			ImGuiStyle& style = ImGui::GetStyle();
		
			if (ImGui::CollapsingHeader("Render settings", ImGuiTreeNodeFlags_DefaultOpen)) {
				float w1 = (ImGui::CalcItemWidth() - style.ItemInnerSpacing.x) / 2.0f;
				float w2 = (ImGui::CalcItemWidth() - style.ItemInnerSpacing.x) / 3.0f;
				ImGui::SliderFloat("Detail Scale", &detailScale, 1.0f,16.0f);
				ImGui::SliderFloat("Detail Strength", &detailStrength, 0.0f, 4.0f);
				ImGui::SliderFloat("Density Scale", &densityScale, 0.0f, 5.0f);
				ImGui::PushItemWidth(w1);
				ImGui::SliderFloat("Sun Azimuth", &azimuth, 0.0f, 360.0f);
				ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
				ImGui::SliderFloat("Sun elevation", &elevation, -10.0f, 90.0f);
				ImGui::PopItemWidth();
				ImGui::SliderFloat3("Light Color", &lightColor.x, 0.0f, 1.0f);
				ImGui::SliderFloat("Light Strength", &lightStrength, 1.0f, 50.0f);
				ImGui::SliderFloat3("Sky Color", &skyColor.x, 0.0f, 1.0f);
				ImGui::SliderFloat("Exposure", &exposure, 0.1f, 1.0f);
				ImGui::SliderFloat("ShadowDensity", &shadowDensity, 0.1f, 1.0f);
				ImGui::SliderFloat("Near Shadow Reach", &nearShadowReach, 0.25f, 4.0f);
				ImGui::SliderFloat("Far Shadow Reach", &farShadowReach, 4.0f, 128.0f);
				ImGui::PushItemWidth(w2);
				ImGui::SliderFloat("forward", &phaserG.x, 0.0f, 0.9f);
				ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
				ImGui::SliderFloat("back", &phaserG.y, -0.9f, 0.0f);
				ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
				ImGui::SliderFloat("mix", &phaserG.z, 0.0f, 1.0f);
				ImGui::PopItemWidth();
				ImGui::SliderFloat("Step size", &stepSize, 0.25f, 4.0f);
				ImGui::SliderInt("Max steps", &maxSteps, 32, 512);
				ImGui::SliderFloat("Powder mix", &powderMix, 0.0f, 1.0f);
				ImGui::Checkbox("Use Jitter", &useJitter);
				ImGui::Checkbox("Use Detail Noise", &useDetailNoise);
			}
		}
	}

private:
	int dimensions = 128;
	Shader m_shader;
	std::vector<ComputePass> m_pipeline;
	enum PassIndex { PassBase = 0, PassDetail = 1 };
	FluidCamera m_camera;
	unsigned int VAO;
	int m_width, m_height;
	float time = 0;

	//3d textures
	GLuint volumeTex;
	GLuint detailTex;
	GLuint frameTex;
	GLuint volumeLoc, detailLoc, frameLoc;

	//Noise variables
	float amplitude = 1.0f;
	float frequency = 0.041f;
	float persistance = 0.5f;
	float lacunarity = 2.0f;
	int octaves = 6;
	float numCells = 3;
	bool timeEffect = false;
	float timeScale = 1.0f;

	float amplitudeDetail = 1.0f;
	float persistanceDetail = 0.5f;
	float lacunarityDetail = 2.0f;
	int octavesDetail = 3;
	float numCellsDetail = 4;
	bool timeEffectDetail = true;
	float timeScaleDetail = 1.0f;

	//Shader variables
	float azimuth = 63.4f, elevation = 27.6f;

	float detailScale = 4.0f;
	float detailStrength = 2.0f;
	float densityScale = 0.45f;
	glm::vec3 lightDir = glm::vec3(0.0f,1.0f,0.0f);
	glm::vec3 lightColor = glm::vec3(0.9f, 0.65f, 0.40f);	
	float lightStrength = 20.0f;
	glm::vec3 skyColor = glm::vec3(0.2f, 0.3f, 0.8f);
	float exposure = 0.9f;
	float shadowDensity = 0.5f;
	glm::vec3 phaserG = glm::vec3(0.6f, -0.2f, 0.3f);
	float nearShadowReach = 0.5f;
	float farShadowReach = 20.0f;
	float stepSize = 0.5f;
	int maxSteps = 256;
	float powderMix = 0.5f;
	glm::vec3 ambientColorTop = glm::vec3(0.2f, 0.3f, 0.8f);
	glm::vec3 ambientColorBottom = glm::vec3(0.1f, 0.1f, 0.1f);
	bool useJitter = true;
	bool useDetailNoise = true;
};