#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Timeline.hpp"
#include <algorithm>
#include <cstdio>
#include <string>
int main(int argc,char**argv){
 double bpm=132.0;for(int i=1;i+1<argc;i++)if(std::string(argv[i])=="--bpm")bpm=std::max(1.0,std::atof(argv[++i]));
 if(!glfwInit()){std::fprintf(stderr,"GLFW initialization failed\n");return 1;}glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,4);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,1);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
 glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
#endif
 GLFWwindow*w=glfwCreateWindow(1440,900,"Impossible Wireframe 3.0",nullptr,nullptr);if(!w){std::fprintf(stderr,"OpenGL 4.1 context creation failed\n");glfwTerminate();return 2;}glfwMakeContextCurrent(w);glfwSwapInterval(1);
 Renderer r;if(!r.init(w)){std::fprintf(stderr,"Renderer init failed: %s\n",r.error().c_str());glfwDestroyWindow(w);glfwTerminate();return 3;}
 Timeline timeline(bpm);SceneSystem scenes;uint64_t seed=0x49574f424a454354ull;int manual=-1,lastScene=-1;bool prevL=false,prevR=false;
 while(!glfwWindowShouldClose(w)){
  double now=glfwGetTime();auto sync=timeline.sample(now);int autoScene=int(sync.barIndex/2)%scenes.count();
  bool L=glfwGetKey(w,GLFW_KEY_LEFT)==GLFW_PRESS,R=glfwGetKey(w,GLFW_KEY_RIGHT)==GLFW_PRESS;if(L&&!prevL)manual=(manual<0?autoScene:manual)-1;if(R&&!prevR)manual=(manual<0?autoScene:manual)+1;prevL=L;prevR=R;if(manual>=0){manual=(manual%scenes.count()+scenes.count())%scenes.count();}if(glfwGetKey(w,GLFW_KEY_SPACE)==GLFW_PRESS)manual=-1;
  int scene=manual<0?autoScene:manual;const Mesh3&m=scenes.mesh(scene,now,seed);if(scene!=lastScene){auto&si=scenes.info(scene);std::fprintf(stdout,"scene %02d: %.*s [%.*s]\n",scene,int(si.name.size()),si.name.data(),int(si.provenance.size()),si.provenance.data());lastScene=scene;}
  if(!r.upload(m)){std::fprintf(stderr,"Mesh rejected in scene %d: %s\n",scene,r.error().c_str());break;}int W,H;glfwGetFramebufferSize(w,&W,&H);glViewport(0,0,W,H);glClearColor(.002f+.008f*sync.pulse,.004f,.009f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);float rad=std::max(.1f,geo::stats(m).radius);r.draw(float(now),H?float(W)/H:1.f,std::min(1.35f,1.85f/rad),1.f+sync.pulse);glfwSwapBuffers(w);glfwPollEvents();if(glfwGetKey(w,GLFW_KEY_ESCAPE)==GLFW_PRESS)glfwSetWindowShouldClose(w,1);
 }
 r.shutdown();glfwDestroyWindow(w);glfwTerminate();return 0;
}
