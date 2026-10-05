#version 410 core
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;

out VertexData { vec3 object; } vertex;

void main(){
  vec4 p=uMVP*vec4(aPos,1.0);
  gl_Position=p;
  vertex.object=aPos;
}
