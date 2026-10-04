#pragma once
#include "Geometry.hpp"
#include <cstddef>
#include <string>
struct GLFWwindow;
class Renderer {
public:
 bool init(GLFWwindow* w,const std::string& shaderDir="shaders");
 bool upload(const Mesh3& m);
 bool draw(float time,int width,int height,float aspect,float scale=1.f,float lineWidth=1.f,float musicLevel=0.f);
 bool initOverlay(const std::string& assetDir="assets/overlay");
 void shutdown();
 const std::string& error() const { return error_; }
private:
 bool initPost(const std::string& shaderDir);
 bool resizeHdr(int width,int height);
 bool drawOverlays(float time,int width,int height,float musicLevel);
 unsigned vao_=0,vbo_=0,ebo_=0,program_=0,postProgram_=0,postVao_=0,hdrFbo_=0,hdrTex_=0,depthRbo_=0;
 unsigned overlayProgram_=0,overlayVao_=0,overlayVbo_=0,logoTex_=0,greetsTex_=0,accentTex_=0,scrollerTex_=0;
 int edgeCount_=0,hdrW_=0,hdrH_=0,logoW_=0,logoH_=0,greetsW_=0,greetsH_=0,accentW_=0,accentH_=0,scrollerW_=0,scrollerH_=0;
 float overlayScrollU_=0.f,overlayLastTime_=-1.f;
 float logoLevel_=0.f,logoLastTime_=-1.f;
 size_t vboCapacity_=0,eboCapacity_=0; int uMVP_=-1,uTime_=-1,uColor_=-1,uMusicLevel_=-1,uPostScene_=-1,uPostTime_=-1,uPostResolution_=-1,uPostMusic_=-1;
 int uOverlayTex_=-1,uOverlayScene_=-1,uOverlayAlpha_=-1,uOverlayTint_=-1,uOverlayTime_=-1,uOverlayFx_=-1;
 std::string error_;
};
