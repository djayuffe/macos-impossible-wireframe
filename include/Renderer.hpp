#pragma once
#include "Geometry.hpp"
#include "BitmapFont.hpp"
#include <cstddef>
#include <string>
struct GLFWwindow;
struct RenderOptions {
 bool bloom=true,background=true,overlays=true,depth=true;
 float exposure=1.f;
};
class Renderer {
public:
 Renderer()=default;
 Renderer(const Renderer&)=delete;
 Renderer& operator=(const Renderer&)=delete;
 RenderOptions options;
 bool init(GLFWwindow* w,const std::string& shaderDir="shaders");
 bool upload(const Mesh3& m);
 bool draw(float time,int width,int height,float aspect,float scale=1.f,float lineWidth=1.f,float musicLevel=0.f);
 bool initOverlay(const std::string& assetDir="assets/overlay",const std::string& shaderDir="shaders");
 void shutdown();
 const std::string& error() const { return error_; }
 const std::string& capabilities() const { return capabilities_; }
 bool checkGpu(const char* stage);
private:
 bool drawBloom();
 bool initPost(const std::string& shaderDir);
 bool resizeHdr(int width,int height);
 bool drawOverlays(float time,int width,int height,float musicLevel,float modelZoom);
 unsigned vao_=0,vbo_=0,ebo_=0,program_=0,postProgram_=0,postVao_=0,hdrFbo_=0,hdrTex_=0,depthRbo_=0;
 unsigned overlayProgram_=0,overlayVao_=0,overlayVbo_=0,logoTex_=0,fontTex_=0;
 int edgeCount_=0,hdrW_=0,hdrH_=0,logoW_=0,logoH_=0,fontW_=0,fontH_=0;
 std::array<bitmapfont::Glyph,36> glyphs_{};
 std::string scrollerText_;
 float scrollPixels_=0.f,overlayLastTime_=-1.f;
 float logoLevel_=0.f,logoLastTime_=-1.f;
 unsigned bloomProgram_=0,bloomFbo_[2]{},bloomTex_[2]{};
 int bloomW_=0,bloomH_=0,maxTargetSize_=0;
 int uViewport_=-1,uLineWidth_=-1,uPostBloom_=-1,uPostBloomOn_=-1,uPostBackground_=-1,uPostExposure_=-1;
 int uBloomDirection_=-1,uBloomExtract_=-1;
 size_t vboCapacity_=0,eboCapacity_=0; int uMVP_=-1,uTime_=-1,uColor_=-1,uMusicLevel_=-1,uPostScene_=-1,uPostTime_=-1,uPostResolution_=-1,uPostMusic_=-1;
 int uOverlayTex_=-1,uOverlayScene_=-1,uOverlayAlpha_=-1,uOverlayTint_=-1,uOverlayTime_=-1,uOverlayFx_=-1;
 int uOverlayRect_=-1,uOverlayResolution_=-1,uOverlayScroller_=-1;
 std::string error_,capabilities_;
};
