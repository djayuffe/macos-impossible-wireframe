#version 410 core
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
out float vDepth;
void main(){ vec4 p=uMVP*vec4(aPos,1.0); gl_Position=p; vDepth=clamp(1.0-abs(p.z/p.w)*0.16,0.15,1.0); }
