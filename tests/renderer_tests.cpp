#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include "Renderer.hpp"
#include "Scene.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <vector>

void require(bool ok,const std::string& what){if(!ok)throw std::runtime_error(what);}
std::vector<float> pixels(int w,int h){
 std::vector<float> p(size_t(w)*h*4);glReadBuffer(GL_BACK);glReadPixels(0,0,w,h,GL_RGBA,GL_FLOAT,p.data());
 for(float x:p)require(std::isfinite(x)&&x>=0&&x<=1,"non-finite or out-of-range framebuffer");
 return p;
}
double difference(const std::vector<float>&a,const std::vector<float>&b){
 require(a.size()==b.size(),"pixel dimensions differ");double d=0;for(size_t i=0;i<a.size();++i)d+=std::abs(a[i]-b[i]);return d/a.size();
}
int main(int argc,char**argv){
 glfwSetErrorCallback([](int n,const char*s){std::fprintf(stderr,"GLFW %d: %s\n",n,s);});
 if(!glfwInit())return 1;
 glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,4);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,1);
 glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
#ifdef __APPLE__
 glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
#endif
 auto*w=glfwCreateWindow(640,400,"GPU validation",nullptr,nullptr);
 if(!w){glfwTerminate();return 1;}
 glfwMakeContextCurrent(w);glfwSwapInterval(0);Renderer r;int result=0;
 try {
  require(r.init(w),r.error());std::printf("GPU: %s\n",r.capabilities().c_str());
  int width=0,height=0;glfwGetFramebufferSize(w,&width,&height);
  auto draw=[&](float t,float scale=1.f,float line=1.5f){require(r.draw(t,width,height,float(width)/height,scale,line,.35f),r.error());auto p=pixels(width,height);require(r.checkGpu("readback"),r.error());return p;};
  r.options.overlays=false;r.options.background=false;SceneSystem scenes;
  for(int id=0;id<scenes.count();++id){
   const auto&m=scenes.mesh(id,2.5,42);require(r.upload(m),r.error());
   auto p=draw(2.5f,std::min(1.48f,1.8f/std::max(.1f,geo::stats(m).radius)));
   float peak=0;for(size_t i=0;i<p.size();i+=4)peak=std::max({peak,p[i],p[i+1],p[i+2]});
   require(m.e.empty()||peak>.2f,"scene "+std::to_string(id)+" has no visible wires");
   std::printf("scene %02d: GPU readback PASS\n",id);
  }
  require(r.upload(geo::cube()),r.error());auto baseline=draw(6);
  r.options.bloom=false;auto noBloom=draw(6);require(difference(baseline,noBloom)>.0001,"bloom has no visible effect");
  auto thick=draw(6,1,8);require(difference(thick,noBloom)>.0001,"line width has no visible effect");
  r.options.depth=false;require(difference(draw(6,1,8),thick)>.000001,"depth mode has no visible effect");r.options.depth=true;
  r.options.background=true;require(difference(draw(6),noBloom)>.001,"background has no visible effect");
  auto noOverlay=draw(6);r.options.overlays=true;require(difference(draw(6),noOverlay)>.001,"overlays have no visible effect");
  r.options.overlays=false;r.options.background=false;r.options.exposure=.2f;auto dark=draw(6);
  r.options.exposure=2;require(difference(draw(6),dark)>.001,"exposure has no visible effect");r.options.exposure=1;
  // A segment crossing the camera exercises homogeneous near-plane clipping.
  require(r.upload(Mesh3{{{0,0,0},{1,1,8}},{{0,1}}}),r.error());draw(0);
  require(r.upload(Mesh3{}),r.error());draw(0);
  require(!r.upload(Mesh3{{{0,0,0}},{{0,4}}}),"invalid mesh accepted");
  require(!r.draw(std::numeric_limits<float>::quiet_NaN(),width,height,1),"NaN time accepted");
  require(!r.draw(0,0,0,1),"zero-sized framebuffer accepted");
  require(!r.draw(0,std::numeric_limits<int>::max(),height,1),"oversized framebuffer accepted");
  require(r.upload(geo::cube()),r.error());
  for(auto size:{std::pair{321,203},std::pair{800,500},std::pair{640,400}}){
   glfwSetWindowSize(w,size.first,size.second);glfwPollEvents();glfwGetFramebufferSize(w,&width,&height);draw(6);
  }
  r.shutdown();r.shutdown();require(r.init(w),r.error());require(r.upload(geo::cube()),r.error());
  const auto& showcase=scenes.mesh(0,6,42);require(r.upload(showcase),r.error());
  r.options=RenderOptions{};auto capture=draw(6,std::min(1.48f,1.8f/std::max(.1f,geo::stats(showcase).radius)));
  if(argc==2){
   std::ofstream f(argv[1],std::ios::binary);f<<"P6\n"<<width<<" "<<height<<"\n255\n";
   for(int y=height-1;y>=0;--y)for(int x=0;x<width;++x)for(int c=0;c<3;++c){unsigned char b=static_cast<unsigned char>(capture[(size_t(y)*width+x)*4+c]*255.f+.5f);f.write(reinterpret_cast<char*>(&b),1);}
   require(bool(f),"capture write failed");
  }
  std::puts("renderer_tests: PASS (51 scenes, effects, clipping, resizing, invalid input, reinitialization)");
 }catch(const std::exception&e){std::fprintf(stderr,"GPU test FAILED: %s\n",e.what());result=1;}
 r.shutdown();glfwDestroyWindow(w);glfwTerminate();return result;
}
