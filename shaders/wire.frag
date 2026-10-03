#version 410 core
in float vDepth;
out vec4 FragColor;
uniform float uTime;
uniform vec3 uColor;
uniform float uMusicLevel;
void main(){ float pulse=.82+.14*sin(uTime*1.7)+uMusicLevel*.38; vec3 neon=uColor+vec3(.25,.08,.35)*uMusicLevel; FragColor=vec4(neon*pulse*vDepth,1.0); }
