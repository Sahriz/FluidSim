#version 430 core

out vec2 uv;


void main() {

    

    const vec2 pos[3] = vec2[](vec2(-1,-1),vec2(3,-1),vec2(-1,3));
    gl_Position = vec4(pos[gl_VertexID],0,1);
    uv = pos[gl_VertexID]*0.5+0.5;
}