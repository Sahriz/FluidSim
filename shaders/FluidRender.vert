#version 430 core
layout(location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

out vec3 normal;
out vec3 worldPos;
out vec3 Position;

uniform float noiseScale;
uniform float scale;
uniform float amplitude;
uniform float sampleScale;
uniform float frequency;
uniform float time;
uniform float persistance;
uniform float lacunarity;

vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v){
  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
           -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod(i, 289.0);
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
  + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

float h(vec2 p){
    float totalNoise = 0;
    float freq = 1.0/frequency;
    float amp = 1.0;
    float maxHeight = 0.0;
    for(int i = 0; i < 4; i++) {
        totalNoise += snoise(p * freq) * amp;
        freq *= lacunarity;
        amp *= persistance;
        maxHeight += amp*0.95;
    }
    totalNoise = totalNoise / maxHeight;
    float noiseNormalized = (totalNoise + 1.0) / 2.0;
    float smoothNoise = pow(noiseNormalized, 2.0);
    return amplitude * smoothNoise;
}



void main() {
    vec3 position = (aPos + vec3(0.0, h(aPos.xz), 0.0)) * scale;
    normal = normalize(-cross(vec3(2.0*sampleScale, h(aPos.xz + vec2(noiseScale, 0.0)) - h(aPos.xz - vec2(noiseScale, 0.0)), 0.0), vec3(0.0, h(aPos.xz + vec2(0.0, noiseScale)) - h(aPos.xz - vec2(0.0, noiseScale)), 2.0*sampleScale)));

    gl_Position = projection  * view * vec4(position, 1.0);
    Position = position;
}