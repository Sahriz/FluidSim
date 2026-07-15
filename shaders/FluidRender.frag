#version 430 core

uniform sampler3D volume;
uniform sampler3D detailNoise;

uniform vec3 camPos;
uniform mat4 invViewProjMat;
uniform int boxMin;
uniform int boxMax;

in vec2 uv;
out vec4 FragColor;

const vec3 lightDir = normalize(vec3(0.6, 0.35, 0.3));
const vec3 lightColor = vec3(16*3, 14.5*3, 12*3);

const float detailScale    = 8.0;   // tiling repeats across the box — make uniform
const float detailStrength = 2.0;   // make uniform, slider
const float densityScale   = 2.0;   // make uniform, slider

float hg(float cosTheta, float g) {
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * 3.14159265 * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}

float sampleDensity(vec3 pos) {
    vec3 uvw = (pos - vec3(boxMin)) / (vec3(boxMax) - vec3(boxMin));
    float density = texture(volume, uvw).r;                        // raw, [0,1]
    float detail  = texture(detailNoise, uvw * detailScale).r;
    density = max(density - detail * detailStrength * (1.0 - density), 0.0);
    return density * densityScale;                                 // scale LAST, once
}

float lightMarch(vec3 pos) {
    float lightStepSize = 2.0;  // step size along the light ray
    float opticalDepth = 0.0;
    for (int i = 0; i < 30; i++) {       // 4-6 steps is plenty
        pos += lightDir * lightStepSize;                              // step along -lightDir (or toward light)s
        opticalDepth += sampleDensity(pos) * lightStepSize;  // accumulate density along the light ray
    }
    return opticalDepth;       // 1 = fully sunlit, 0 = deep shadow
}

void main() {
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 worldPos = invViewProjMat * vec4(ndc, 1.0, 1.0);
    worldPos /= worldPos.w;

    vec3 rayOrigin = camPos;
    vec3 rayDir = normalize(worldPos.xyz - camPos);

    vec3 t1 = (vec3(boxMin) - rayOrigin) / rayDir;
    vec3 t2 = (vec3(boxMax) - rayOrigin) / rayDir;
    vec3 tsmall = min(t1, t2);
    vec3 tbig   = max(t1, t2);
    float tEnter = max(max(tsmall.x, tsmall.y), tsmall.z);
    tEnter = max(tEnter, 0.0);
    float tExit  = min(min(tbig.x, tbig.y), tbig.z);

    if(tExit < 0.0 || tEnter > tExit) { FragColor = vec4(0.2, 0.3, 0.8, 1.0); return; }

    float stepSize = 0.5;                              // total distance inside box / numSteps
    int   numSteps = int((tExit - tEnter) / stepSize) + 1;  // limit to 100 steps for performance
    if(numSteps > 256){
        numSteps = 256;
        stepSize = (tExit - tEnter) / 256.0;
    }
    vec3  color = vec3(0.0);
    float transmittance = 1.0;
    const vec3 skyColor = vec3(0.2, 0.3, 0.8);

    float cosTheta = dot(rayDir, lightDir);
    float phase = mix(hg(cosTheta, 0.6), hg(cosTheta, -0.2), 0.3);   // forward lobe + mild 

    float jitter = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);
    for (int i = 0; i < numSteps; i++) {

        float t = tEnter + (float(i) + jitter) * stepSize;   // replaces the fixed +0.5
        vec3 pos  = rayOrigin + t * rayDir;
        vec3 uvw  = (pos - vec3(boxMin)) / (vec3(boxMax) - vec3(boxMin)); 
        
        float density = sampleDensity(pos);
        if(density > 0.001) {
            float tau = lightMarch(pos) * 0.1;   // shadowing
            float sunTerm = exp(-tau) + 0.35 * exp(-tau * 0.25) + 0.12 * exp(-tau * 0.06);

            float powder = 1.0 - exp(-2.0 * tau);  // Beer-Lambert law
            sunTerm *= mix(1.0, powder, 0.5);  // add some powdery scattering to the sun

            vec3 ambient = mix(vec3(0.08, 0.10, 0.14), vec3(0.22, 0.30, 0.45), uvw.y);   
            vec3 sampleColor = lightColor * sunTerm * phase + ambient;

            float slabTrans  = exp(-density * stepSize);      // this slab's own opacity
            color           += sampleColor * (1.0 - slabTrans) * transmittance;
            transmittance   *= slabTrans;

        }  
        if(transmittance < 0.01) break;  // early exit if almost opaque
    }
    color = vec3(1.0) - exp(-color * 0.6);  // convert from optical depth to color
    color = pow(color, vec3(1.0 / 2.2));
    FragColor = vec4(color + skyColor * transmittance, 1.0);
}