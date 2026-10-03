#pragma once
#include "Geometry.hpp"
#include <cstddef>
#include <string>
struct GLFWwindow;
class Renderer {
public:
 bool init(GLFWwindow* w,const std::string& shaderDir="shaders");
 bool upload(const Mesh3& m);
 void draw(float time,float aspect,float scale=1.f,float lineWidth=1.f);
 void shutdown();
 const std::string& error() const { return error_; }
private:
 unsigned vao_=0,vbo_=0,ebo_=0,program_=0; int edgeCount_=0;
 size_t vboCapacity_=0,eboCapacity_=0; int uMVP_=-1,uTime_=-1,uColor_=-1;
 std::string error_;
};
