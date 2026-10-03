#version 410 core
in float vDepth;
out vec4 FragColor;
uniform float uTime;
uniform vec3 uColor;
void main(){ float pulse=.86+.14*sin(uTime*1.7); FragColor=vec4(uColor*pulse*vDepth,1.0); }
