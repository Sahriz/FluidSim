#pragma once

#include "keyHandler.h"
#include "Shader.h"
#include "Camera.h"
#include "TerrainCamera.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class Scene {
public:
	Scene(std::string title) : m_title(title){}
	virtual ~Scene() = default;

	virtual void onEnter() = 0;
	virtual void onExit() = 0;

	virtual void update(float dt, frameInput& in) = 0;
	virtual void render() = 0;

	virtual void drawUI() = 0;

	std::string m_title;
};