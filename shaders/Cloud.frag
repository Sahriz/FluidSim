#version 460 core

layout(std140, binding = 0) uniform Camera{
    mat4 view;
    mat4 projection;
    mat4 viewProj;
    mat4 invViewProjMat;
    vec4 camPos;
};

uniform sampler3D volume;
uniform sampler3D detailNoise;

uniform int boxMin;
uniform int boxMax;

uniform float detailScale;
uniform float detailStrength;
uniform float densityScale;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform float lightStrength;
uniform vec3 skyColor;

uniform float shadowDensity;
uniform vec3 phaserG;
uniform float nearShadowReach;
uniform float farShadowReach;
uniform float stepSize;
uniform int maxSteps;
uniform float powderMix;

uniform vec3 ambientColorTop;
uniform vec3 ambientColorBottom;
uniform int useJitter;
uniform int useDetailNoise;

in vec2 uv;
out vec4 FragColor;

float hg(float cosTheta, float g) {
    float g2 = g * g;
    float base = max(1.0 + g2 - 2.0 * g * cosTheta, 1e-4);
    return (1.0 - g2) / (4.0 * 3.14159265 * pow(base, 1.5));
}

vec3 toUvw(vec3 pos){
    return (pos - vec3(boxMin)) / (vec3(boxMax) - vec3(boxMin));
}

float scatter(float tau, float cosTheta){
    float a = 1.0, b = 1.0, c = 1.0;
    float sum = 0.0;
    for(int o = 0; o < 3; o++){
        float ph = mix(hg(cosTheta, phaserG.x * c), hg(cosTheta, phaserG.y * c), phaserG.z);
        sum += b * exp(-tau * a) * ph;
        a *= 0.25; b *= 0.7; c *= 0.5;
    }
    return sum;
}

float sampleDensity(vec3 pos) {
    vec3 uvw = (pos - vec3(boxMin)) / (vec3(boxMax) - vec3(boxMin));
    float density = texture(volume, uvw).r;                        // raw, [0,1]
    float detail  = texture(detailNoise, uvw * detailScale).r;
    if(useDetailNoise == 1) density = max(density - detail * detailStrength * (1.0 - density), 0.0);
    return density * densityScale;                                 // scale LAST, once
}

bool outsideBox(vec3 pos){
    return any(lessThan(pos, vec3(boxMin))) || any(greaterThan(pos,vec3(boxMax)));
}

float lightMarch(vec3 pos) {
    float opticalDepth = 0.0;

    float stepN = nearShadowReach/ 3.0f;  // step size along the light ray
    
    for (int i = 0; i < 3; i++) {       // 4-6 steps is plenty
        pos += lightDir * stepN;                              // step along -lightDir (or toward light)s
        
        if(outsideBox(pos)){ return opticalDepth; }
        opticalDepth += sampleDensity(pos) * stepN;  // accumulate density along the light ray
    }

    float stepF = (farShadowReach - nearShadowReach) / 4.0;
    for(int i = 0; i < 4; i ++){
        pos += lightDir * stepF;
        if(outsideBox(pos)) break;
        opticalDepth += texture(volume, toUvw(pos)).r * densityScale * stepF;
        if(opticalDepth * shadowDensity > 6.0) break;
    }
    return opticalDepth;       // 1 = fully sunlit, 0 = deep shadow
}

void main() {
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 worldPos = invViewProjMat * vec4(ndc, 1.0, 1.0);
    worldPos /= worldPos.w;

    vec3 rayOrigin = camPos.xyz;
    vec3 rayDir = normalize(worldPos.xyz - camPos.xyz);

    vec3 t1 = (vec3(boxMin) - rayOrigin) / rayDir;
    vec3 t2 = (vec3(boxMax) - rayOrigin) / rayDir;
    vec3 tsmall = min(t1, t2);
    vec3 tbig   = max(t1, t2);
    float tEnter = max(max(tsmall.x, tsmall.y), tsmall.z);
    tEnter = max(tEnter, 0.0);
    float tExit  = min(min(tbig.x, tbig.y), tbig.z);

    if(tExit < 0.0 || tEnter > tExit) { FragColor = vec4(skyColor,1.0); return; }
    float step = stepSize;
    int numSteps = int((tExit - tEnter) / step) + 1;  // limit to 100 steps for performance
    if(numSteps > maxSteps){
        numSteps = maxSteps;
        step = (tExit - tEnter) / float(maxSteps);
    }
    vec3  color = vec3(0.0);
    float transmittance = 1.0;
    

    float cosTheta = dot(rayDir, lightDir);
    float phase = mix(hg(cosTheta, phaserG.x), hg(cosTheta, phaserG.y), phaserG.z);   // forward lobe + mild 

    float jitter = useJitter == 1 ? fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453) : 1.0;
    for (int i = 0; i < numSteps; i++) {

        float t = tEnter + (float(i) + jitter) * step;   // replaces the fixed +0.5
        vec3 pos  = rayOrigin + t * rayDir;
        vec3 uvw  = toUvw(pos); 
        
        float density = sampleDensity(pos);
        if(density > 0.01 && transmittance > 0.05) {
            float tau = lightMarch(pos) * shadowDensity;   // shadowing
            float sunTerm = scatter(tau, cosTheta);

            float powder = 1.0 - exp(-2.0 * tau);  // Beer-Lambert law
            float litView = clamp(-cosTheta * 0.5 + 0.5, 0.0, 1.0);

            sunTerm *= mix(1.0, powder, powderMix * litView);  // add some powdery scattering to the sun

            vec3 ambient = mix(ambientColorBottom, ambientColorTop, uvw.y);   
            vec3 sampleColor = lightColor * lightStrength * sunTerm + ambient;

            float slabTrans  = exp(-density * step);      // this slab's own opacity
            color           += sampleColor * (1.0 - slabTrans) * transmittance;
            transmittance   *= slabTrans;

        }  
        if(transmittance < 0.01) break;  // early exit if almost opaque
    }
    
    
    FragColor = vec4(color + skyColor * transmittance, 1.0);
}