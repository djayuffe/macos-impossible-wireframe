#include "Renderer.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
namespace {std::string readText(const std::string&p){std::ifstream f(p);if(!f)return {};std::ostringstream s;s<<f.rdbuf();return s.str();}
bool shader(GLuint&out,GLenum type,const std::string&s,std::string&err){out=glCreateShader(type);const char*p=s.c_str();glShaderSource(out,1,&p,nullptr);glCompileShader(out);GLint ok=0;glGetShaderiv(out,GL_COMPILE_STATUS,&ok);if(!ok){GLint n=0;glGetShaderiv(out,GL_INFO_LOG_LENGTH,&n);std::string log(std::max(1,n),'\0');glGetShaderInfoLog(out,n,nullptr,log.data());err=log;glDeleteShader(out);out=0;return false;}return true;}
void perspective(float*m,float fovy,float aspect,float zn,float zf){float f=1/std::tan(fovy*.5f);for(int i=0;i<16;i++)m[i]=0;m[0]=f/aspect;m[5]=f;m[10]=(zf+zn)/(zn-zf);m[11]=-1;m[14]=(2*zf*zn)/(zn-zf);}
void mul(float*o,const float*a,const float*b){float r[16]{};for(int c=0;c<4;c++)for(int rr=0;rr<4;rr++)for(int k=0;k<4;k++)r[c*4+rr]+=a[k*4+rr]*b[c*4+k];std::copy(r,r+16,o);}}
bool Renderer::init(GLFWwindow*,const std::string&dir){std::string vs=readText(dir+"/wire.vert"),fs=readText(dir+"/wire.frag");if(vs.empty()||fs.empty()){error_="cannot read shaders from "+dir;return false;}GLuint v=0,f=0;if(!shader(v,GL_VERTEX_SHADER,vs,error_)||!shader(f,GL_FRAGMENT_SHADER,fs,error_)){if(v)glDeleteShader(v);return false;}program_=glCreateProgram();glAttachShader(program_,v);glAttachShader(program_,f);glLinkProgram(program_);glDeleteShader(v);glDeleteShader(f);GLint ok=0;glGetProgramiv(program_,GL_LINK_STATUS,&ok);if(!ok){GLint n=0;glGetProgramiv(program_,GL_INFO_LOG_LENGTH,&n);error_.resize(std::max(1,n));glGetProgramInfoLog(program_,n,nullptr,error_.data());return false;}uMVP_=glGetUniformLocation(program_,"uMVP");uTime_=glGetUniformLocation(program_,"uTime");uColor_=glGetUniformLocation(program_,"uColor");glGenVertexArrays(1,&vao_);glGenBuffers(1,&vbo_);glGenBuffers(1,&ebo_);glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);return true;}
bool Renderer::upload(const Mesh3&m){std::string why;if(!geo::validate(m,&why)){error_=why;return false;}std::vector<uint32_t>ix;ix.reserve(m.e.size()*2);for(auto e:m.e){ix.push_back(e.a);ix.push_back(e.b);}edgeCount_=(int)ix.size();glBindVertexArray(vao_);glBindBuffer(GL_ARRAY_BUFFER,vbo_);size_t vb=m.v.size()*sizeof(V3);if(vb>vboCapacity_){vboCapacity_=std::max(vb,vboCapacity_*2+4096);glBufferData(GL_ARRAY_BUFFER,vboCapacity_,nullptr,GL_DYNAMIC_DRAW);}if(vb)glBufferSubData(GL_ARRAY_BUFFER,0,vb,m.v.data());glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(V3),nullptr);glEnableVertexAttribArray(0);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo_);size_t eb=ix.size()*sizeof(uint32_t);if(eb>eboCapacity_){eboCapacity_=std::max(eb,eboCapacity_*2+4096);glBufferData(GL_ELEMENT_ARRAY_BUFFER,eboCapacity_,nullptr,GL_DYNAMIC_DRAW);}if(eb)glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,0,eb,ix.data());return true;}
void Renderer::draw(float t,float aspect,float scale,float lineWidth){float P[16],V[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,-4.2f,1},R[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};float c=std::cos(t*.17f),s=std::sin(t*.17f);R[0]=c*scale;R[2]=-s*scale;R[8]=s*scale;R[10]=c*scale;R[5]=scale;float X[16],M[16];perspective(P,55.f*3.14159265f/180.f,std::max(.05f,aspect),.05f,100.f);mul(X,V,R);mul(M,P,X);glUseProgram(program_);glUniformMatrix4fv(uMVP_,1,GL_FALSE,M);glUniform1f(uTime_,t);glUniform3f(uColor_,.82f,.92f,1.f);glLineWidth(std::max(1.f,lineWidth));glBindVertexArray(vao_);glDrawElements(GL_LINES,edgeCount_,GL_UNSIGNED_INT,nullptr);}
void Renderer::shutdown(){if(program_)glDeleteProgram(program_);if(ebo_)glDeleteBuffers(1,&ebo_);if(vbo_)glDeleteBuffers(1,&vbo_);if(vao_)glDeleteVertexArrays(1,&vao_);program_=vao_=vbo_=ebo_=0;}
