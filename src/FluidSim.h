#pragma once

#include <iostream>
#include "Scene.h"
#include <vector>
#include "FluidCamera.h"
#include <fstream>
#include <sstream>

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

	std::string readFile(const std::string& filePath) {
		std::ifstream file(filePath);
		std::stringstream buffer;
		if (file) {
			buffer << file.rdbuf();
		}
		else {
			std::cerr << "Failed to open file: " << filePath << "\n";
		}
		return buffer.str();
	}
	
	GLuint CreateComputeShaderProgram(const std::string& path) {
		GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
		std::string source = readFile(path);
		if (source.empty()) {
			std::cerr << "Shader source is empty: " << path << "\n";
			glDeleteShader(shader);
			return 0;
		}
		const char* src = source.c_str();
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		// Check compilation status
		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLint logLength;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
			std::vector<char> log(logLength);
			glGetShaderInfoLog(shader, logLength, nullptr, log.data());
			std::cerr << "Compute Shader compilation failed:\n" << log.data() << std::endl;
			glDeleteShader(shader);
			return 0;
		}
		// Link shader into a program
		GLuint program = glCreateProgram();
		glAttachShader(program, shader);
		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			GLint logLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
			std::vector<char> log(logLength);
			glGetProgramInfoLog(program, logLength, nullptr, log.data());
			std::cerr << "Program linking failed:\n" << log.data() << std::endl;
			glDeleteShader(shader);
			glDeleteProgram(program);
			return 0;
		}
		glDeleteShader(shader); // Safe to delete after linking
		return program;
	}

	void InitNoiseUniforms() {
		amplitudeLoc = glGetUniformLocation(m_noiseShader, "amplitude");
		frequencyLoc = glGetUniformLocation(m_noiseShader, "frequency");
		persistanceLoc = glGetUniformLocation(m_noiseShader, "persistance");
		lacunarityLoc = glGetUniformLocation(m_noiseShader, "lacunarity");
		octaveLoc = glGetUniformLocation(m_noiseShader, "octaves");
		dimensionLoc = glGetUniformLocation(m_noiseShader, "dimension");
		timeLoc = glGetUniformLocation(m_noiseShader, "time");
		typeNoiseLoc = glGetUniformLocation(m_noiseShader, "typeOfNoise");
		numCellsLoc = glGetUniformLocation(m_noiseShader, "numCells");
		timeScaleLoc = glGetUniformLocation(m_noiseShader, "timeScale");
		timeCheckBoxLoc = glGetUniformLocation(m_noiseShader, "useTimeScale");
	}

	void InitShaderUniforms() {

		detailScaleLoc = glGetUniformLocation(m_shader.ID, "detailScale");
		detailStrengthLoc = glGetUniformLocation(m_shader.ID, "detailStrength");
		densityScaleLoc = glGetUniformLocation(m_shader.ID, "densityScale");
		lightDirLoc = glGetUniformLocation(m_shader.ID, "lightDir");
		lightColorLoc = glGetUniformLocation(m_shader.ID, "lightColor");
		lightStrengthLoc = glGetUniformLocation(m_shader.ID, "lightStrength");
		skyColorLoc = glGetUniformLocation(m_shader.ID, "skyColor");
		exposureLoc = glGetUniformLocation(m_shader.ID, "exposure");
		shadowDensityLoc = glGetUniformLocation(m_shader.ID, "shadowDensity");
		phaserGLoc = glGetUniformLocation(m_shader.ID, "phaserG");
		shadowReachLoc = glGetUniformLocation(m_shader.ID, "shadowReach");
		stepSizeLoc = glGetUniformLocation(m_shader.ID, "stepSize");
		maxStepsLoc = glGetUniformLocation(m_shader.ID, "maxSteps");
		powderMixLoc = glGetUniformLocation(m_shader.ID, "powderMix");
		AmbientColorTopLoc = glGetUniformLocation(m_shader.ID, "ambientColorTop");
		AmbientColorBottomLoc = glGetUniformLocation(m_shader.ID, "ambientColorBottom");
		useJitterLoc = glGetUniformLocation(m_shader.ID, "useJitter");
		useDetailNoiseLoc = glGetUniformLocation(m_shader.ID, "useDetailNoise");
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
	}

	void InitShaders() {
		m_noiseShader = CreateComputeShaderProgram(std::string(SHADER_DIR) + "Noise.comp");
	}

	void createDetail() {
		glUseProgram(m_noiseShader);
		glBindImageTexture(0, detailTex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

		glUniform1f(amplitudeLoc, amplitudeDetail);
		glUniform1f(lacunarityLoc, lacunarityDetail);
		glUniform1f(persistanceLoc, persistanceDetail);
		glUniform1i(dimensionLoc, dimensions/4);
		glUniform1f(timeLoc, time);
		glUniform1i(typeNoiseLoc, 1);
		glUniform1f(numCellsLoc, numCellsDetail);
		glUniform1f(timeScaleLoc, timeScaleDetail);
		glUniform1i(timeCheckBoxLoc, timeEffectDetail ? 1 : 0);

		int N = dimensions / (8 * 4);
		glDispatchCompute(N, N, N);

		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}

	void createNoise() {
		glUseProgram(m_noiseShader);
		glBindImageTexture(0, volumeTex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

		glUniform1f(amplitudeLoc, amplitude);
		glUniform1f(frequencyLoc, frequency);
		glUniform1f(lacunarityLoc, lacunarity);
		glUniform1f(persistanceLoc, persistance);
		glUniform1i(octaveLoc, octaves);
		glUniform1i(dimensionLoc,dimensions);
		glUniform1f(timeLoc, time);
		glUniform1i(typeNoiseLoc, 0);
		glUniform1f(numCellsLoc, numCells);
		glUniform1f(timeScaleLoc, timeScale);
		glUniform1i(timeCheckBoxLoc, timeEffect ? 1 : 0);
		
		int N = dimensions / 8;
		glDispatchCompute(N, N, N);

		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}

	void onEnter() override {
		std::string vertPath = std::string(SHADER_DIR) + "FluidRender.vert";
		std::string fragPath = std::string(SHADER_DIR) + "FluidRender.frag";

		m_shader.init(vertPath, fragPath);

		InitProgram();
		glUseProgram(m_shader.ID);
		float aspectRatio = float(m_width) / float(m_height);
		m_camera.Init(70, aspectRatio, 0.1, 1000, m_shader.ID, dimensions);
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
		InitShaders();
		InitNoiseUniforms();
		InitShaderUniforms();
		InitTextures();
		createDetail();
		createNoise();
	}

	void onExit() override {
		glDeleteVertexArrays(1, &VAO);
		glDeleteTextures(1, &volumeTex);
		glDeleteTextures(1, &detailTex);
	}

	void update(float dt, frameInput& in) override {
		
		m_camera.Update(in);
		time += dt;

		
	}

	void updateUniforms() {
		glUseProgram(m_shader.ID);
		glUniform1f(detailScaleLoc, detailScale);
		glUniform1f(detailStrengthLoc, detailStrength);
		glUniform1f(densityScaleLoc, densityScale);
		glUniform3f(lightDirLoc, lightDir.x, lightDir.y, lightDir.z);
		glUniform3f(lightColorLoc, lightColor.x, lightColor.y, lightColor.z);
		glUniform1f(lightStrengthLoc, lightStrength);
		glUniform3f(skyColorLoc, skyColor.x, skyColor.y, skyColor.z);
		glUniform1f(exposureLoc, exposure);
		glUniform1f(shadowDensityLoc, shadowDensity);
		glUniform3f(phaserGLoc, phaserG.x, phaserG.y, phaserG.z);
		glUniform1f(shadowReachLoc, shadowReach);
		glUniform1f(stepSizeLoc, stepSize);
		glUniform1i(maxStepsLoc, maxSteps);
		glUniform1f(powderMixLoc, powderMix);
		glUniform3f(AmbientColorTopLoc, ambientColorTop.x, ambientColorTop.y, ambientColorTop.z);
		glUniform3f(AmbientColorBottomLoc, ambientColorBottom.x, ambientColorBottom.y, ambientColorBottom.z);
		glUniform1i(useJitterLoc, useJitter ? 1 : 0);
		glUniform1i(useDetailNoiseLoc, useDetailNoise ? 1 : 0);
	}

	void updateLightDirection() {
		float az = glm::radians(azimuth), el = glm::radians(elevation);
		lightDir = glm::vec3(cos(el) * sin(az), sin(el), cos(el) * cos(az));
	}

	void render() override {
		glClearColor(0.2f, 0.3f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		createNoise();
		createDetail();
		updateUniforms();
		updateLightDirection();
		glUseProgram(m_shader.ID);
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
		
		if (ImGui::CollapsingHeader("Density Noise Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SliderFloat("Amplitude", &amplitude, 0.0f, 5.0f);
			ImGui::SliderFloat("Frequency", &frequency, 0.0f, 0.1f);
			ImGui::SliderFloat("Persistance", &persistance, 0.0f, 1.0f);
			ImGui::SliderFloat("Lacunarity", &lacunarity, 0.0f, 5.0f);
			ImGui::SliderInt("Octaves", &octaves, 1, 10);
			ImGui::SliderFloat("Number of Cells", &numCells, 2.0f, 64.0f);
			ImGui::SliderFloat("Time Scale", &timeScale, 0.0f, 25.0f);
			ImGui::Checkbox("Time Effect", &timeEffect);
		}
		
		ImGui::Separator();
		
		//Second section
		if (ImGui::CollapsingHeader("Detail Noise Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SliderFloat("Detail Amplitude", &amplitudeDetail, 16, 256);
			ImGui::SliderFloat("Detail Persistance", &persistanceDetail, 0.0f, 1.0f);
			ImGui::SliderFloat("Detail Lacunarity", &lacunarityDetail, 0.0f, 5.0f);
			ImGui::SliderFloat("Detail Number of Cells", &numCellsDetail, 2.0f, 64.0f);
			ImGui::SliderFloat("Detail Time Scale", &timeScaleDetail, 0.0f, 25.0f);
			ImGui::Checkbox("Detail Time Effect", &timeEffectDetail);
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
			ImGui::SliderFloat("Shadow Reach", &shadowReach, 5.0f, 128.0f);
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

private:
	int dimensions = 128;
	Shader m_shader;
	FluidCamera m_camera;
	unsigned int VAO;
	int m_width, m_height;
	float time = 0;

	//3d textures
	GLuint volumeTex;
	GLuint detailTex;
	GLuint volumeLoc, detailLoc;

	//Noise variables
	GLuint m_noiseShader, m_detailShader;
	GLuint amplitudeLoc, frequencyLoc, persistanceLoc, lacunarityLoc, octaveLoc, dimensionLoc, timeLoc, typeNoiseLoc, numCellsLoc, timeScaleLoc, timeCheckBoxLoc;

	float amplitude = 1.0f;
	float frequency = 0.008f;
	float persistance = 0.5f;
	float lacunarity = 2.0f;
	int octaves = 6;
	float numCells = 3;
	bool timeEffect = false;
	float timeScale = 1.0f;

	float amplitudeDetail = 1.0f;
	float persistanceDetail = 0.5f;
	float lacunarityDetail = 2.0f;
	float numCellsDetail = 4;
	bool timeEffectDetail = true;
	float timeScaleDetail = 1.0f;

	//Shader variables
	GLuint detailScaleLoc, detailStrengthLoc, densityScaleLoc, lightDirLoc, lightColorLoc, lightStrengthLoc, skyColorLoc,
		exposureLoc, shadowDensityLoc, phaserGLoc, shadowReachLoc, stepSizeLoc, maxStepsLoc, powderMixLoc,
		AmbientColorTopLoc, AmbientColorBottomLoc, useJitterLoc, useDetailNoiseLoc;

	float azimuth = 63.4f, elevation = 27.6f;

	float detailScale = 4.0f;
	float detailStrength = 2.0f;
	float densityScale = 1.0f;
	glm::vec3 lightDir = glm::vec3(0.0f,1.0f,0.0f);
	glm::vec3 lightColor = glm::vec3(0.9f, 0.65f, 0.40f);	
	float lightStrength = 20.0f;
	glm::vec3 skyColor = glm::vec3(0.2f, 0.3f, 0.8f);
	float exposure = 0.9f;
	float shadowDensity = 0.5f;
	glm::vec3 phaserG = glm::vec3(0.6f, -0.2f, 0.3f);
	float shadowReach = 15.0f;
	float stepSize = 0.5f;
	int maxSteps = 256;
	float powderMix = 0.5f;
	glm::vec3 ambientColorTop = glm::vec3(0.2f, 0.3f, 0.8f);
	glm::vec3 ambientColorBottom = glm::vec3(0.1f, 0.1f, 0.1f);
	bool useJitter = true;
	bool useDetailNoise = true;
};