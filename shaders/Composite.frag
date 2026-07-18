#version 460 core

uniform sampler2D frame;

uniform float exposure;

in vec2 uv;
out vec4 FragColor;

void main() {
    vec3 hdr = texture(frame, uv).rgb;
    vec3 color = vec3(1.0) - exp(-hdr * exposure);
    color = pow(color, vec3(1.0/2.2));
    float n = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);
    color += (n - 0.5) / 255.0;
    FragColor = vec4(color, 1.0);
}