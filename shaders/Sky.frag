#version 460 core

layout(std140, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 viewProj;
    mat4 invViewProjMat;
    vec4 camPos;
};

uniform vec3 lightDir;
uniform vec3 zenithColor;
uniform vec3 horizonColor;
uniform vec3 sunColor;
uniform float sunIntensity;
uniform float sunSize;
uniform float glowStrength;
uniform float glowFalloff;

in vec2 uv;
out vec4 FragColor;

void main() {
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 worldPos = invViewProjMat * vec4(ndc, 1.0, 1.0);
    worldPos /= worldPos.w;
    vec3 rayDir = normalize(worldPos.xyz - camPos.xyz);

    //Vertical gradient
    float horizonness = pow(1.0 - clamp(rayDir.y, 0.0, 1.0), 4.0);
    vec3 sky = mix(zenithColor, horizonColor, horizonness);

    //sun glow
    float cosSun = dot(rayDir, lightDir);
    sky += sunColor * glowStrength * pow(max(cosSun, 0.0), glowFalloff);

    //sun disk
    float disc = smoothstep(sunSize - 0.0002, sunSize, cosSun);
    sky += sunColor * sunIntensity * disc;

    //below the horizon: fade towards a ground tone so flying low doesn't look broken
    sky = mix(sky, horizonColor * 0.4, smoothstep(0.0, -0.15, rayDir.y));

    FragColor = vec4(sky, 1.0);
}