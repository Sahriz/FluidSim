#version 430 core

in vec3 normal;
in vec3 worldPos;
in vec3 Position;
out vec4 FragColor;

void main() {
    vec3 normalizedNormal = normalize(normal);

    vec3 lightDir = normalize(vec3(0.3, 1.0, 0.5));
    float diff = max(dot(normalizedNormal, lightDir),0.0);
    vec3 albedo = vec3(0.0,0.0,0.0);

    vec3 waterColor = vec3(0.05, 0.15, 0.3);
    vec3 grassColor = vec3(0.1, 0.5, 0.1);
    vec3 snowColor = vec3(0.85, 0.85, 0.85);
    vec3 rockColor = vec3(0.25, 0.25, 0.25);

    float waterGrassFactor = smoothstep(0.0, 0.15, Position.y);
    vec3 terrainColor = mix(waterColor, grassColor, waterGrassFactor);

    float grassSnowFactor = smoothstep(0.65, 0.70, Position.y);
    terrainColor = mix(terrainColor, snowColor, grassSnowFactor);

    float slopeFactor = smoothstep(0.55, 0.65, normalizedNormal.y);
    vec3 finalAlbedo = mix(rockColor, terrainColor, slopeFactor);

    vec3 diffuse = diff * finalAlbedo;
    FragColor = vec4(pow(diffuse,vec3(1.0/2.2)), 1.0);
}