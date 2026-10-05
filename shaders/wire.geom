#version 410 core
layout(lines) in;
layout(triangle_strip, max_vertices=4) out;
in VertexData { vec3 object; } vertices[];
out float vDepth;
out vec3 vObject;
out vec2 vNdc;
noperspective out float vAcross;
uniform vec2 uViewport;
uniform float uLineWidth;

void emitWire(vec4 p, vec3 object, vec2 offset, float side) {
  gl_Position=p+vec4(offset*p.w,0.,0.);
  vObject=object;
  vNdc=p.xy/p.w;
  vDepth=clamp(1.-abs(p.z/p.w)*.16,.15,1.);
  vAcross=side;
  EmitVertex();
}
void main() {
  vec4 a=gl_in[0].gl_Position,b=gl_in[1].gl_Position;
  vec3 oa=vertices[0].object,ob=vertices[1].object;
  // Clip against the near plane BEFORE dividing by w. Lines crossing the
  // camera must not turn into full-screen spikes or non-finite coordinates.
  float da=a.z+a.w,db=b.z+b.w;
  if(da<0. && db<0.) return;
  if(da<0.) { float t=da/(da-db); a=mix(a,b,t); oa=mix(oa,ob,t); }
  else if(db<0.) { float t=db/(db-da); b=mix(b,a,t); ob=mix(ob,oa,t); }
  if(a.w<=1e-6 || b.w<=1e-6) return;
  vec2 delta=(b.xy/b.w-a.xy/a.w)*uViewport;
  float len=length(delta);
  if(len<1e-5) return;
  vec2 normal=vec2(-delta.y,delta.x)/len;
  // An additional pixel on each side provides analytic edge coverage.
  vec2 offset=normal*(uLineWidth+2.)/uViewport;
  float halfWidth=uLineWidth*.5+1.;
  emitWire(a,oa, offset, halfWidth);
  emitWire(a,oa,-offset,-halfWidth);
  emitWire(b,ob, offset, halfWidth);
  emitWire(b,ob,-offset,-halfWidth);
  EndPrimitive();
}
