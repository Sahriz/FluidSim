#version 430 core

uniform sampler3D volume;
uniform sampler3D detailNoise;

uniform vec3 camPos;
uniform mat4 invViewProjMat;
uniform int boxMin;
uniform int boxMax;

uniform float detailScale;
uniform float detailStrength;
uniform float densityScale;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform float lightStrength;
uniform vec3 skyColor;

uniform float exposure;
uniform float shadowDensity;
uniform vec3 phaserG;
uniform float shadowReach;
uniform float stepSize;
uniform int maxSteps;
uniform float powderMix;

uniform vec3 ambientColorTop;
uniform vec3 ambientColorBottom;
uniform int useJitter;
uniform int useDetailNoise;

in vec2 uv;
out vec4 FragColor;


vec3 finalizeColor(vec3 linear){
    vec3 finalColor = vec3(1.0) - exp(-linear * exposure);  // convert from optical depth to color
    finalColor = pow(finalColor, vec3(1.0 / 2.2));
    return finalColor;
}

float hg(float cosTheta, float g) {
    float g2 = g * g;
    float base = max(1.0 + g2 - 2.0 * g * cosTheta, 1e-4);
    return (1.0 - g2) / (4.0 * 3.14159265 * pow(base, 1.5));
}

float sampleDensity(vec3 pos) {
    vec3 uvw = (pos - vec3(boxMin)) / (vec3(boxMax) - vec3(boxMin));
    float density = texture(volume, uvw).r;                        // raw, [0,1]
    float detail  = texture(detailNoise, uvw * detailScale).r;
    if(useDetailNoise == 1) density = max(density - detail * detailStrength * (1.0 - density), 0.0);
    return density * densityScale;                                 // scale LAST, once
}

float lightMarch(vec3 pos) {
    float lightStepSize = shadowReach/6.0f;  // step size along the light ray
    float opticalDepth = 0.0;
    for (int i = 0; i < 6; i++) {       // 4-6 steps is plenty
        pos += lightDir * lightStepSize;                              // step along -lightDir (or toward light)s
        if((any(lessThan(pos, vec3(boxMin))) || any(greaterThan(pos,vec3(boxMax))))){ break; }
        opticalDepth += sampleDensity(pos) * lightStepSize;  // accumulate density along the light ray
        if(opticalDepth * shadowDensity > 6.0) { break; }
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

    if(tExit < 0.0 || tEnter > tExit) { FragColor = vec4(finalizeColor(skyColor),1.0); return; }
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
        vec3 uvw  = (pos - vec3(boxMin)) / (vec3(boxMax) - vec3(boxMin)); 
        
        float density = sampleDensity(pos);
        if(density > 0.01 && transmittance > 0.05) {
            float tau = lightMarch(pos) * shadowDensity;   // shadowing
            float sunTerm = exp(-tau) + 0.35 * exp(-tau * 0.25) + 0.12 * exp(-tau * 0.06);

            float powder = 1.0 - exp(-2.0 * tau);  // Beer-Lambert law
            sunTerm *= mix(1.0, powder, powderMix);  // add some powdery scattering to the sun

            vec3 ambient = mix(vec3(0.08, 0.10, 0.14), vec3(0.22, 0.30, 0.45), uvw.y);   
            vec3 sampleColor = lightColor * lightStrength * sunTerm * phase + ambient;

            float slabTrans  = exp(-density * step);      // this slab's own opacity
            color           += sampleColor * (1.0 - slabTrans) * transmittance;
            transmittance   *= slabTrans;

        }  
        if(transmittance < 0.01) break;  // early exit if almost opaque
    }
    vec3 finalColor = finalizeColor(color + skyColor * transmittance);
    
    FragColor = vec4(finalColor, 1.0);
}