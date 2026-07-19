#version 460 core

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 viewProj;
    mat4 invViewProjMat;
    vec4 camPos;
};

uniform vec3 skyColor;

in vec2 uv;
out vec4 FragColor;

void main() {
    FragColor = vec4(vec3(skyColor),1.0);
}