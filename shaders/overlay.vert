#version 410 core
layout(location=0) in vec2 p;
layout(location=1) in vec2 uv;
out vec2 vUv;
uniform float uTime;
uniform float uFx;
uniform vec4 uRect;
uniform vec2 uResolution;
uniform bool uScroller;
void main() {
  vec2 q=p;
  if(!uScroller) {
    // Rotate in pixel space about this card's centre (not the screen origin).
    vec2 local=(p-uRect.xy)*uResolution;
    float angle=sin(uTime*.58)*.045*uFx;
    float cs=cos(angle),sn=sin(angle);
    local=mat2(cs,-sn,sn,cs)*local;
    q=uRect.xy+local/uResolution;
  } else {
    q.y+=.012*sin(p.x*7.5-uTime*2.2);
  }
  vUv=uv;gl_Position=vec4(q,0.,1.);
}
