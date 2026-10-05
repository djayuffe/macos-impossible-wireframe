#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Timeline.hpp"
#include "Audio.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace {
void help(){
 std::puts("Impossible Wireframe: OpenGL 4.1 GPU demo\n"
 "Usage: impossible_wireframe [--bpm 1..1000] [--music FILE] [--no-music]\n"
 "  [--scene 0..50] [--frames N] [--no-bloom] [--no-background] [--no-overlays]\n"
 "  [--xray] [--exposure 0.1..4] [--line-width 1..12]\n"
 "Keys: arrows scenes; Space automatic; B bloom; G background; O overlays;\n"
 "      D depth; +/- exposure; [/] line width; Escape quit.\n"
 "All GPU effects enabled by default. --frames performs a bounded test run.");
}
bool number(const char* text,double lo,double hi,double& value){
 try{std::string s(text);size_t end=0;value=std::stod(s,&end);return end==s.size()&&std::isfinite(value)&&value>=lo&&value<=hi;}catch(...){return false;}
}
}
int main(int argc,char**argv){
 double bpm=132.;int manual=-1,frameLimit=0;float lineWidth=1.25f;
 bool noMusic=false;RenderOptions options;std::filesystem::path musicPath;
 for(int i=1;i<argc;++i){
  std::string arg(argv[i]);double n=0;
  if(arg=="--help"||arg=="-h"){help();return 0;}
  if(arg=="--no-music")noMusic=true;
  else if(arg=="--no-bloom")options.bloom=false;
  else if(arg=="--no-background")options.background=false;
  else if(arg=="--no-overlays")options.overlays=false;
  else if(arg=="--xray")options.depth=false;
  else if(arg=="--music"&&i+1<argc)musicPath=std::filesystem::absolute(argv[++i]);
  else if(arg=="--bpm"&&i+1<argc&&number(argv[i+1],1,1000,n)){bpm=n;++i;}
  else if(arg=="--exposure"&&i+1<argc&&number(argv[i+1],.1,4,n)){options.exposure=float(n);++i;}
  else if(arg=="--line-width"&&i+1<argc&&number(argv[i+1],1,12,n)){lineWidth=float(n);++i;}
  else if(arg=="--scene"&&i+1<argc&&number(argv[i+1],0,50,n)&&n==std::floor(n)){manual=int(n);++i;}
  else if(arg=="--frames"&&i+1<argc&&number(argv[i+1],1,1000000,n)&&n==std::floor(n)){frameLimit=int(n);++i;}
  else{std::fprintf(stderr,"Unknown option or invalid value: %s\n",arg.c_str());help();return 2;}
 }
 // Resolve copied runtime resources even when started from another directory.
 if(!std::filesystem::exists("shaders/wire.vert")||!std::filesystem::exists("assets/overlay/logo.rgba")){
  std::error_code ec;auto executable=std::filesystem::absolute(argv[0],ec);
#ifdef __APPLE__
  uint32_t len=0;_NSGetExecutablePath(nullptr,&len);std::string path(len,'\0');
  if(_NSGetExecutablePath(path.data(),&len)==0)executable=path.c_str();
#elif defined(__linux__)
  executable=std::filesystem::read_symlink("/proc/self/exe",ec);
#endif
  auto dir=std::filesystem::weakly_canonical(executable,ec).parent_path();
  if(!ec)std::filesystem::current_path(dir,ec);
  if(ec){std::fprintf(stderr,"Cannot locate runtime resources: %s\n",ec.message().c_str());return 2;}
 }
 if(musicPath.empty())musicPath="assets/music/drozerix_-_silicon_dancer.mod";
 glfwSetErrorCallback([](int code,const char* message){std::fprintf(stderr,"GLFW %d: %s\n",code,message);});
 if(!glfwInit())return 1;
 glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,4);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,1);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
 glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GLFW_TRUE);
#endif
 GLFWwindow*w=glfwCreateWindow(1440,900,"macOS Impossible Wireframe - GPU",nullptr,nullptr);
 if(!w){glfwTerminate();return 2;}
 glfwMakeContextCurrent(w);glfwSwapInterval(1);
 Renderer r;r.options=options;
 if(!r.init(w)){std::fprintf(stderr,"Renderer init failed: %s\n",r.error().c_str());r.shutdown();glfwDestroyWindow(w);glfwTerminate();return 3;}
 std::printf("GPU: %s\n",r.capabilities().c_str());
 AudioPlayer audio;
#if defined(IW_HAS_AUDIO)
 if(!noMusic&&SDL_InitSubSystem(SDL_INIT_AUDIO)!=0)std::fprintf(stderr,"SDL audio init failed: %s\n",SDL_GetError());
#endif
 if(!noMusic&&audio.open(musicPath))std::printf("music: %s\n",musicPath.string().c_str());
 Timeline timeline(bpm);SceneSystem scenes;uint64_t lastRevision=0;
 int lastScene=-1,frames=0,result=0;float radius=1.f;
 std::array<bool,GLFW_KEY_LAST+1> wasDown{};
 auto pressed=[&](int key){bool down=glfwGetKey(w,key)==GLFW_PRESS;bool edge=down&&!wasDown[key];wasDown[key]=down;return edge;};
 while(!glfwWindowShouldClose(w)){
  glfwPollEvents();if(pressed(GLFW_KEY_ESCAPE))break;
  auto music=audio.state();double seconds=music.active?music.seconds:glfwGetTime();
  auto sync=timeline.sample(seconds);sync.pulse=std::max(sync.pulse,music.level);
  int automatic=int(sync.barIndex/2%scenes.count());
  if(pressed(GLFW_KEY_LEFT))manual=((manual<0?automatic:manual)+scenes.count()-1)%scenes.count();
  if(pressed(GLFW_KEY_RIGHT))manual=((manual<0?automatic:manual)+1)%scenes.count();
  if(pressed(GLFW_KEY_SPACE))manual=-1;
  if(pressed(GLFW_KEY_B))r.options.bloom=!r.options.bloom;
  if(pressed(GLFW_KEY_G))r.options.background=!r.options.background;
  if(pressed(GLFW_KEY_O))r.options.overlays=!r.options.overlays;
  if(pressed(GLFW_KEY_D))r.options.depth=!r.options.depth;
  if(pressed(GLFW_KEY_EQUAL))r.options.exposure=std::min(4.f,r.options.exposure+.1f);
  if(pressed(GLFW_KEY_MINUS))r.options.exposure=std::max(.1f,r.options.exposure-.1f);
  if(pressed(GLFW_KEY_LEFT_BRACKET))lineWidth=std::max(1.f,lineWidth-.25f);
  if(pressed(GLFW_KEY_RIGHT_BRACKET))lineWidth=std::min(12.f,lineWidth+.25f);
  int width=0,height=0;glfwGetFramebufferSize(w,&width,&height);
  if(width<=0||height<=0){glfwWaitEventsTimeout(.05);continue;}
  int scene=manual<0?automatic:manual;
  const Mesh3&m=scenes.mesh(scene,seconds,0x49574f424a454354ull);
  if(scenes.revision()!=lastRevision){
   if(!r.upload(m)){result=4;break;}
   radius=std::max(.1f,geo::stats(m).radius);lastRevision=scenes.revision();
  }
  if(scene!=lastScene){auto& si=scenes.info(scene);std::printf("scene %02d: %.*s [%.*s]\n",scene,int(si.name.size()),si.name.data(),int(si.provenance.size()),si.provenance.data());lastScene=scene;}
  float sizeCycle=1.f+.11f*std::sin(float(seconds)*.41f+float(scene)*.37f)+.07f*sync.pulse;
  if(!r.draw(float(seconds),width,height,float(width)/height,std::min(1.48f,1.92f/radius)*sizeCycle,lineWidth+sync.pulse,music.level)){result=5;break;}
  glfwSwapBuffers(w);++frames;
  if(frames%60==0){
   char title[256];std::snprintf(title,sizeof(title),"Impossible Wireframe | scene %02d | bloom %s | depth %s | exposure %.1f | width %.2f",scene,r.options.bloom?"on":"off",r.options.depth?"on":"off",r.options.exposure,lineWidth);glfwSetWindowTitle(w,title);
  }
  if(frameLimit&&frames>=frameLimit)break;
 }
 if(result)std::fprintf(stderr,"Rendering failed: %s\n",r.error().c_str());
 r.shutdown();audio.close();
#if defined(IW_HAS_AUDIO)
 SDL_QuitSubSystem(SDL_INIT_AUDIO);
#endif
 glfwDestroyWindow(w);glfwTerminate();return result;
}
