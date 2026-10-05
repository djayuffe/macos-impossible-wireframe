#include "Renderer.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdint>
#include <limits>
#include <cctype>
namespace {std::string readText(const std::string&p){std::ifstream f(p);if(!f)return {};std::ostringstream s;s<<f.rdbuf();return s.str();}
bool shader(GLuint&out,GLenum type,const std::string&s,std::string&err){out=glCreateShader(type);const char*p=s.c_str();glShaderSource(out,1,&p,nullptr);glCompileShader(out);GLint ok=0;glGetShaderiv(out,GL_COMPILE_STATUS,&ok);if(!ok){GLint n=0;glGetShaderiv(out,GL_INFO_LOG_LENGTH,&n);std::string log(std::max(1,n),'\0');glGetShaderInfoLog(out,n,nullptr,log.data());err=log;glDeleteShader(out);out=0;return false;}return true;}
GLuint linkProgram(const std::string& vs,const std::string& fs,std::string& err,const std::string& gs={}) {
 GLuint v=0,f=0,g=0;
 bool ok=shader(v,GL_VERTEX_SHADER,vs,err)&&shader(f,GL_FRAGMENT_SHADER,fs,err);
 if(ok&&!gs.empty()) ok=shader(g,GL_GEOMETRY_SHADER,gs,err);
 GLuint p=0;
 if(ok) {
  p=glCreateProgram();glAttachShader(p,v);glAttachShader(p,f);if(g)glAttachShader(p,g);
  glLinkProgram(p);GLint linked=0;glGetProgramiv(p,GL_LINK_STATUS,&linked);
  if(!linked){GLint n=0;glGetProgramiv(p,GL_INFO_LOG_LENGTH,&n);err.resize(std::max(1,n));glGetProgramInfoLog(p,n,nullptr,err.data());glDeleteProgram(p);p=0;}
 }
 if(v)glDeleteShader(v);
 if(f)glDeleteShader(f);
 if(g)glDeleteShader(g);
 return p;
}
void perspective(float*m,float fovy,float aspect,float zn,float zf){float f=1/std::tan(fovy*.5f);for(int i=0;i<16;i++)m[i]=0;m[0]=f/aspect;m[5]=f;m[10]=(zf+zn)/(zn-zf);m[11]=-1;m[14]=(2*zf*zn)/(zn-zf);}
void mul(float*o,const float*a,const float*b){float r[16]{};for(int c=0;c<4;c++)for(int rr=0;rr<4;rr++)for(int k=0;k<4;k++)r[c*4+rr]+=a[k*4+rr]*b[c*4+k];std::copy(r,r+16,o);}struct RawImage{int w=0,h=0;std::vector<unsigned char> rgba;};
bool readRawImage(const std::string&path,RawImage&img,std::string&err){
 std::ifstream f(path,std::ios::binary|std::ios::ate);
 if(!f){err="cannot read overlay texture "+path;return false;}
 auto size=f.tellg();f.seekg(0);unsigned char header[8]{};f.read(reinterpret_cast<char*>(header),8);
 auto le32=[&](int i){return uint32_t(header[i])|(uint32_t(header[i+1])<<8)|(uint32_t(header[i+2])<<16)|(uint32_t(header[i+3])<<24);};
 uint32_t w=le32(0),h=le32(4);GLint limit=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&limit);
 if(!f||!w||!h||w>uint32_t(limit)||h>uint32_t(limit)||uint64_t(w)*h*4+8!=uint64_t(size)||uint64_t(w)*h*4>256*1024*1024){err="invalid, oversized or truncated overlay texture "+path;return false;}
 img.w=int(w);img.h=int(h);img.rgba.resize(size_t(w)*h*4);f.read(reinterpret_cast<char*>(img.rgba.data()),std::streamsize(img.rgba.size()));
 if(!f){err="cannot read overlay pixels "+path;return false;}return true;
}
bool makeTexture(const std::string&path,GLuint&tex,int&w,int&h,std::string&err){RawImage img;if(!readRawImage(path,img,err))return false;if(!tex)glGenTextures(1,&tex);glBindTexture(GL_TEXTURE_2D,tex);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,img.w,img.h,0,GL_RGBA,GL_UNSIGNED_BYTE,img.rgba.data());glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);glGenerateMipmap(GL_TEXTURE_2D);w=img.w;h=img.h;return true;}
float smoother(float a,float b,float x){x=std::clamp((x-a)/(b-a),0.f,1.f);return x*x*(3.f-2.f*x);}}
bool Renderer::checkGpu(const char* stage){
 GLenum e=glGetError();if(e==GL_NO_ERROR)return true;
 std::ostringstream s;s<<stage<<": OpenGL error 0x"<<std::hex<<e;
 while((e=glGetError())!=GL_NO_ERROR)s<<", 0x"<<e;
 error_=s.str();return false;
}
bool Renderer::init(GLFWwindow* window,const std::string&dir){
 if(!window||glfwGetCurrentContext()!=window){error_="renderer needs its current OpenGL context";return false;}
 shutdown();error_.clear();
 GLint major=0,minor=0,rbo=0;glGetIntegerv(GL_MAJOR_VERSION,&major);glGetIntegerv(GL_MINOR_VERSION,&minor);
 if(major<4||(major==4&&minor<1)){error_="OpenGL 4.1 core is required";return false;}
 glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxTargetSize_);glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE,&rbo);maxTargetSize_=std::min(maxTargetSize_,rbo);
 capabilities_=std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER)))+" | "+reinterpret_cast<const char*>(glGetString(GL_VERSION))+" | max target "+std::to_string(maxTargetSize_);
 std::string vs=readText(dir+"/wire.vert"),fs=readText(dir+"/wire.frag"),gs=readText(dir+"/wire.geom");
 if(vs.empty()||fs.empty()||gs.empty()){error_="cannot read wire shaders from "+dir;return false;}
 program_=linkProgram(vs,fs,error_,gs);if(!program_)return false;
 uMVP_=glGetUniformLocation(program_,"uMVP");uTime_=glGetUniformLocation(program_,"uTime");uColor_=glGetUniformLocation(program_,"uColor");uMusicLevel_=glGetUniformLocation(program_,"uMusicLevel");
 uViewport_=glGetUniformLocation(program_,"uViewport");uLineWidth_=glGetUniformLocation(program_,"uLineWidth");
 glGenVertexArrays(1,&vao_);glGenBuffers(1,&vbo_);glGenBuffers(1,&ebo_);glGenVertexArrays(1,&postVao_);
 if(!initPost(dir)||!initOverlay("assets/overlay",dir))return false;
 glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);glDisable(GL_FRAMEBUFFER_SRGB);
 return checkGpu("renderer initialization");
}
bool Renderer::initPost(const std::string&dir){
 std::string vs=readText(dir+"/post.vert"),fs=readText(dir+"/post.frag"),bloom=readText(dir+"/bloom.frag");
 if(vs.empty()||fs.empty()||bloom.empty()){error_="cannot read post shaders from "+dir;return false;}
 postProgram_=linkProgram(vs,fs,error_);if(!postProgram_)return false;
 uPostScene_=glGetUniformLocation(postProgram_,"uScene");uPostTime_=glGetUniformLocation(postProgram_,"uTime");uPostResolution_=glGetUniformLocation(postProgram_,"uResolution");uPostMusic_=glGetUniformLocation(postProgram_,"uMusicLevel");
 uPostBloom_=glGetUniformLocation(postProgram_,"uBloom");uPostBloomOn_=glGetUniformLocation(postProgram_,"uBloomOn");uPostBackground_=glGetUniformLocation(postProgram_,"uBackground");uPostExposure_=glGetUniformLocation(postProgram_,"uExposure");
 glUseProgram(postProgram_);glUniform1i(uPostScene_,0);glUniform1i(uPostBloom_,1);
 bloomProgram_=linkProgram(vs,bloom,error_);if(!bloomProgram_)return false;
 glUseProgram(bloomProgram_);glUniform1i(glGetUniformLocation(bloomProgram_,"uSource"),0);
 uBloomDirection_=glGetUniformLocation(bloomProgram_,"uDirection");uBloomExtract_=glGetUniformLocation(bloomProgram_,"uExtract");return true;
}
bool Renderer::initOverlay(const std::string&dir,const std::string&shaderDir){
 const std::string vs=readText(shaderDir+"/overlay.vert"),fs=readText(shaderDir+"/overlay.frag");
 if(vs.empty()||fs.empty()){error_="cannot read overlay shaders from "+shaderDir;return false;}
 overlayProgram_=linkProgram(vs,fs,error_);if(!overlayProgram_)return false;
 glGenVertexArrays(1,&overlayVao_);glGenBuffers(1,&overlayVbo_);
 uOverlayTex_=glGetUniformLocation(overlayProgram_,"uTex");uOverlayScene_=glGetUniformLocation(overlayProgram_,"uScene");uOverlayAlpha_=glGetUniformLocation(overlayProgram_,"uAlpha");uOverlayTint_=glGetUniformLocation(overlayProgram_,"uTint");uOverlayTime_=glGetUniformLocation(overlayProgram_,"uTime");uOverlayFx_=glGetUniformLocation(overlayProgram_,"uFx");
 uOverlayRect_=glGetUniformLocation(overlayProgram_,"uRect");uOverlayResolution_=glGetUniformLocation(overlayProgram_,"uResolution");uOverlayScroller_=glGetUniformLocation(overlayProgram_,"uScroller");
 glUseProgram(overlayProgram_);if(uOverlayTex_>=0)glUniform1i(uOverlayTex_,0);if(uOverlayScene_>=0)glUniform1i(uOverlayScene_,1);
 if(!makeTexture(dir+"/logo.rgba",logoTex_,logoW_,logoH_,error_))return false;
 RawImage source;if(!readRawImage(dir+"/font.rgba",source,error_))return false;
 try{
  auto atlas=bitmapfont::crop(source.rgba,source.w,source.h);
  glyphs_=atlas.glyphs;fontW_=atlas.width;fontH_=atlas.height;
  glGenTextures(1,&fontTex_);glBindTexture(GL_TEXTURE_2D,fontTex_);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,fontW_,fontH_,0,GL_RGBA,GL_UNSIGNED_BYTE,atlas.pixels.data());
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  // 16-pixel gutters remain isolated through the supported mip levels.
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,3);glGenerateMipmap(GL_TEXTURE_2D);
 }catch(const std::exception& e){error_=e.what();return false;}
 scrollerText_=readText(dir+"/scroller.txt");
 if(scrollerText_.empty()||scrollerText_.size()>4096){error_="scroller.txt must contain 1 to 4096 characters";return false;}
 for(char& c:scrollerText_){
  c=char(std::toupper(static_cast<unsigned char>(c)));
  if(std::isspace(static_cast<unsigned char>(c)))c=' ';
  if(c!=' '&&bitmapfont::index(c)<0){error_="scroller.txt supports letters A-Z, digits 0-9 and spaces";return false;}
 }
 return checkGpu("font atlas upload");
}
bool Renderer::resizeHdr(int width,int height){
 if(width<=0||height<=0||width>maxTargetSize_||height>maxTargetSize_){error_="framebuffer dimensions exceed GPU limits";return false;}
 if(width==hdrW_&&height==hdrH_&&hdrFbo_)return true;
 hdrW_=hdrH_=0; // Commit dimensions only after ALL attachments are complete.
 if(!hdrFbo_)glGenFramebuffers(1,&hdrFbo_);
 if(!hdrTex_)glGenTextures(1,&hdrTex_);
 if(!depthRbo_)glGenRenderbuffers(1,&depthRbo_);
 if(!bloomFbo_[0])glGenFramebuffers(2,bloomFbo_);
 if(!bloomTex_[0])glGenTextures(2,bloomTex_);
 bloomW_=std::max(1,(width+1)/2);bloomH_=std::max(1,(height+1)/2);
 glActiveTexture(GL_TEXTURE0);
 auto target=[&](GLuint fbo,GLuint tex,int w,int h,bool depth){
  glBindTexture(GL_TEXTURE_2D,tex);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F,w,h,0,GL_RGBA,GL_FLOAT,nullptr);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,tex,0);
  if(depth){glBindRenderbuffer(GL_RENDERBUFFER,depthRbo_);glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,w,h);glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depthRbo_);}
  return glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;
 };
 bool ok=target(hdrFbo_,hdrTex_,width,height,true)&&target(bloomFbo_[0],bloomTex_[0],bloomW_,bloomH_,false)&&target(bloomFbo_[1],bloomTex_[1],bloomW_,bloomH_,false);
 glBindFramebuffer(GL_FRAMEBUFFER,0);
 if(!ok){error_="HDR/bloom framebuffer incomplete";return false;}
 if(!checkGpu("framebuffer allocation"))return false;
 hdrW_=width;hdrH_=height;return true;
}
bool Renderer::upload(const Mesh3&m){
 if(!program_){error_="upload before renderer initialization";return false;}
 std::string why;if(!geo::validate(m,&why)){error_=why;return false;}
 if(m.e.size()>size_t(std::numeric_limits<GLsizei>::max())/2){error_="mesh exceeds GPU index count";return false;}
 std::vector<uint32_t>ix;ix.reserve(m.e.size()*2);for(auto e:m.e){ix.push_back(e.a);ix.push_back(e.b);}
 glBindVertexArray(vao_);glBindBuffer(GL_ARRAY_BUFFER,vbo_);
 size_t vb=m.v.size()*sizeof(V3);vboCapacity_=std::max(vb,vboCapacity_);
 // Orphan in-use storage so a deforming mesh need not wait on last frame.
 glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(vboCapacity_),nullptr,GL_STREAM_DRAW);
 if(vb)glBufferSubData(GL_ARRAY_BUFFER,0,GLsizeiptr(vb),m.v.data());
 glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(V3),nullptr);glEnableVertexAttribArray(0);
 glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo_);size_t eb=ix.size()*sizeof(uint32_t);eboCapacity_=std::max(eb,eboCapacity_);
 glBufferData(GL_ELEMENT_ARRAY_BUFFER,GLsizeiptr(eboCapacity_),nullptr,GL_STREAM_DRAW);
 if(eb)glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,0,GLsizeiptr(eb),ix.data());
 if(!checkGpu("mesh upload"))return false;
 edgeCount_=int(ix.size());return true;
}
bool Renderer::drawBloom(){
 glUseProgram(bloomProgram_);glBindVertexArray(postVao_);glViewport(0,0,bloomW_,bloomH_);glActiveTexture(GL_TEXTURE0);
 for(int pass=0;pass<4;++pass){
  int target=pass%2;glBindFramebuffer(GL_FRAMEBUFFER,bloomFbo_[target]);
  glBindTexture(GL_TEXTURE_2D,pass==0?hdrTex_:bloomTex_[1-target]);
  glUniform1i(uBloomExtract_,pass==0);glUniform2f(uBloomDirection_,target==0?1.f/bloomW_:0.f,target==1?1.f/bloomH_:0.f);
  glDrawArrays(GL_TRIANGLES,0,3);
 }
 return true;
}
bool Renderer::draw(float t,int width,int height,float aspect,float scale,float lineWidth,float musicLevel){
 if(!program_||!std::isfinite(t)||!std::isfinite(aspect)||!std::isfinite(scale)||!std::isfinite(lineWidth)||!std::isfinite(musicLevel)||!std::isfinite(options.exposure)||aspect<=0||scale<=0){error_="invalid render parameters";return false;}
 if(!resizeHdr(width,height))return false;
 float ml=std::clamp(musicLevel,0.f,1.f);
 float fly=std::sin(t*.19f),zoom=std::sin(t*.23f+1.7f),breath=1.f+.075f*std::sin(t*.83f)+.12f*ml;
 float P[16],V[16]={1,0,0,0,0,1,0,0,0,0,1,0,.22f*fly,.14f*std::cos(t*.13f),-4.25f+.52f*zoom-.28f*ml,1};
 float Ry[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},Rx[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},R[16],X[16],M[16];
 float cy=std::cos(t*.17f),sy=std::sin(t*.17f),cx=std::cos(.27f*std::sin(t*.11f)),sx=std::sin(.27f*std::sin(t*.11f));
 Ry[0]=cy*scale*breath;Ry[2]=-sy*scale*breath;Ry[8]=sy*scale*breath;Ry[10]=cy*scale*breath;Ry[5]=scale*breath;
 Rx[5]=cx;Rx[6]=sx;Rx[9]=-sx;Rx[10]=cx;
 perspective(P,(52.f+3.f*std::sin(t*.07f)-2.f*ml)*3.14159265f/180.f,std::max(.05f,aspect),.05f,100.f);
 mul(R,Ry,Rx);mul(X,V,R);mul(M,P,X);
 glBindFramebuffer(GL_FRAMEBUFFER,hdrFbo_);glViewport(0,0,width,height);glDepthMask(GL_TRUE);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 if(options.depth)glEnable(GL_DEPTH_TEST);else glDisable(GL_DEPTH_TEST);
 glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
 glUseProgram(program_);glUniformMatrix4fv(uMVP_,1,GL_FALSE,M);glUniform1f(uTime_,t);glUniform1f(uMusicLevel_,ml);
 glUniform3f(uColor_,1.2f+.75f*ml,1.55f+.35f*ml,2.1f+1.1f*ml);
 glUniform2f(uViewport_,float(width),float(height));glUniform1f(uLineWidth_,std::clamp(lineWidth+ml*.75f,1.f,12.f));
 glBindVertexArray(vao_);glDrawElements(GL_LINES,edgeCount_,GL_UNSIGNED_INT,nullptr);
 glDisable(GL_BLEND);glDisable(GL_DEPTH_TEST);
 if(options.bloom)drawBloom();
 glBindFramebuffer(GL_FRAMEBUFFER,0);glViewport(0,0,width,height);glUseProgram(postProgram_);
 glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,bloomTex_[1]);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,hdrTex_);
 glUniform1f(uPostTime_,t);glUniform2f(uPostResolution_,float(width),float(height));glUniform1f(uPostMusic_,ml);
 glUniform1i(uPostBloomOn_,options.bloom);glUniform1i(uPostBackground_,options.background);glUniform1f(uPostExposure_,std::clamp(options.exposure,.1f,4.f));
 glBindVertexArray(postVao_);glDrawArrays(GL_TRIANGLES,0,3);
 float overlayModelZoom=std::clamp(scale*breath*(1.f+.12f*zoom),.72f,1.45f);
 if(options.overlays&&!drawOverlays(t,width,height,ml,overlayModelZoom))return false;
 return checkGpu("frame rendering");
}
bool Renderer::drawOverlays(float t,int width,int height,float musicLevel,float modelZoom){
 if(!overlayProgram_||!logoTex_||!fontTex_)return true;
 auto quad=[&](GLuint tex,float cx,float cy,float w,float h,float alpha,float u0,float u1,float tintR,float tintG,float tintB,float fx){
  if(alpha<=0.f||w<=0.f||h<=0.f)return;
  float x0=cx-w*.5f,x1=cx+w*.5f,y0=cy-h*.5f,y1=cy+h*.5f;
  float v[]={x0,y0,u0,1.f,x1,y0,u1,1.f,x1,y1,u1,0.f,x0,y0,u0,1.f,x1,y1,u1,0.f,x0,y1,u0,0.f};
  glUseProgram(overlayProgram_);glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,hdrTex_);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,tex);if(uOverlayAlpha_>=0)glUniform1f(uOverlayAlpha_,alpha);if(uOverlayTint_>=0)glUniform3f(uOverlayTint_,tintR,tintG,tintB);if(uOverlayTime_>=0)glUniform1f(uOverlayTime_,t);if(uOverlayFx_>=0)glUniform1f(uOverlayFx_,fx);
  glUniform4f(uOverlayRect_,cx,cy,w,h);glUniform2f(uOverlayResolution_,float(width),float(height));glUniform1i(uOverlayScroller_,0);
  glBindVertexArray(overlayVao_);glBindBuffer(GL_ARRAY_BUFFER,overlayVbo_);glBufferData(GL_ARRAY_BUFFER,sizeof(v),v,GL_STREAM_DRAW);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),nullptr);glEnableVertexAttribArray(0);glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),reinterpret_cast<void*>(2*sizeof(float)));glEnableVertexAttribArray(1);glDrawArrays(GL_TRIANGLES,0,6);
 };
 glDisable(GL_DEPTH_TEST);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
 float aspect=width>0&&height>0?float(width)/float(height):1.6f;
 // Smooth the decoded RMS before using it for scale: attack quickly on a hit,
 // release more slowly so the logo feels heavy instead of jittering.
 if(logoLastTime_<0.f||t<logoLastTime_||t-logoLastTime_>1.f) logoLastTime_=t;
 else {float dt=std::clamp(t-logoLastTime_,0.f,.08f);float target=std::clamp(musicLevel,0.f,1.f);float rate=target>logoLevel_?12.f:4.5f;logoLevel_+=(target-logoLevel_)*(1.f-std::exp(-rate*dt));logoLastTime_=t;}
 float travel=.5f+.5f*std::sin(t*.62f-1.1f); // 0 = receding into the wireframe, 1 = foreground hit
 float pulse=.84f+.28f*travel+.035f*std::sin(t*4.2f)+.22f*logoLevel_+.06f*logoLevel_*std::sin(t*9.1f);
 float logoFade=1.f-smoother(22.f,31.f,t);
 float intro=smoother(.15f,1.4f,t)*(1.f-.55f*smoother(10.f,18.f,t));
 float logoW=.94f*modelZoom;float logoH=logoW*(float(logoH_)/float(std::max(1,logoW_)))*aspect;float logoY=.49f+.035f*std::sin(t*.62f-1.1f);quad(logoTex_,0.f,logoY,logoW*pulse,logoH*pulse,std::min(1.f,intro+.22f)*logoFade,0.f,1.f,1.12f,1.04f,1.f,.92f+.08f*logoLevel_);
 // Lay out native-resolution glyphs, each with its own crop and advance.
 // A constant source-pixel clock preserves speed across message edits/resizes.
 float textWidth=0.f;
 for(char c:scrollerText_){int i=bitmapfont::index(c);textWidth+=i<0?95.f:float(glyphs_[i].advance);}
 if(overlayLastTime_>=0.f&&t>=overlayLastTime_&&t-overlayLastTime_<=1.f)
  scrollPixels_=std::fmod(scrollPixels_+(t-overlayLastTime_)*228.75f,textWidth);
 overlayLastTime_=t;
 const float sy=.16f/183.f,sx=sy/aspect;
 std::vector<float> vertices;vertices.reserve(8192);
 float x=-1.f-scrollPixels_*sx;size_t character=0;
 while(x<1.02f){
  char c=scrollerText_[character++%scrollerText_.size()];int index=bitmapfont::index(c);
  if(index<0){x+=95.f*sx;continue;}
  const auto& g=glyphs_[index];float glyphWidth=g.width*sx;
  if(x+glyphWidth> -1.02f){
   float y0=-.9f,y1=y0+g.height*sy;
   float v0=float(g.y+g.height)/fontH_,v1=float(g.y)/fontH_;
   auto vertex=[&](float px,float py,float u,float v){vertices.insert(vertices.end(),{px,py,u,v});};
   // Subdivision lets the vertex shader bend a letter, without sampling outside
   // its crop or clipping its top and bottom as UV displacement used to do.
   for(int slice=0;slice<12;++slice){
    float a=float(slice)/12.f,b=float(slice+1)/12.f;
    float xa=x+glyphWidth*a,xb=x+glyphWidth*b;
    float ua=(g.x+g.width*a)/fontW_,ub=(g.x+g.width*b)/fontW_;
    vertex(xa,y0,ua,v0);vertex(xb,y0,ub,v0);vertex(xb,y1,ub,v1);
    vertex(xa,y0,ua,v0);vertex(xb,y1,ub,v1);vertex(xa,y1,ua,v1);
   }
  }
  x+=g.advance*sx;
 }
 glUseProgram(overlayProgram_);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,fontTex_);
 glUniform1i(uOverlayScroller_,1);glUniform1f(uOverlayAlpha_,1.f);glUniform1f(uOverlayFx_,.25f);
 glUniform1f(uOverlayTime_,t);glUniform3f(uOverlayTint_,1.f,1.f,1.f);glUniform2f(uOverlayResolution_,float(width),float(height));
 glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,hdrTex_);glActiveTexture(GL_TEXTURE0);
 glBindVertexArray(overlayVao_);glBindBuffer(GL_ARRAY_BUFFER,overlayVbo_);
 glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(vertices.size()*sizeof(float)),vertices.data(),GL_STREAM_DRAW);
 glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),nullptr);glEnableVertexAttribArray(0);
 glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),reinterpret_cast<void*>(2*sizeof(float)));glEnableVertexAttribArray(1);
 glDrawArrays(GL_TRIANGLES,0,GLsizei(vertices.size()/4));
 glDisable(GL_BLEND);return true;
}
void Renderer::shutdown(){
 for(GLuint tex:{logoTex_,fontTex_,hdrTex_,bloomTex_[0],bloomTex_[1]})if(tex)glDeleteTextures(1,&tex);
 for(GLuint fbo:{hdrFbo_,bloomFbo_[0],bloomFbo_[1]})if(fbo)glDeleteFramebuffers(1,&fbo);
 for(GLuint buf:{vbo_,ebo_,overlayVbo_})if(buf)glDeleteBuffers(1,&buf);
 for(GLuint vao:{vao_,postVao_,overlayVao_})if(vao)glDeleteVertexArrays(1,&vao);
 for(GLuint p:{program_,postProgram_,overlayProgram_,bloomProgram_})if(p)glDeleteProgram(p);
 if(depthRbo_)glDeleteRenderbuffers(1,&depthRbo_);
 program_=postProgram_=overlayProgram_=bloomProgram_=vao_=postVao_=overlayVao_=vbo_=ebo_=overlayVbo_=hdrFbo_=hdrTex_=depthRbo_=logoTex_=fontTex_=0;
 for(int i=0;i<2;++i){bloomFbo_[i]=bloomTex_[i]=0;}
 hdrW_=hdrH_=bloomW_=bloomH_=edgeCount_=0;vboCapacity_=eboCapacity_=0;
 logoW_=logoH_=fontW_=fontH_=0;scrollerText_.clear();
 scrollPixels_=logoLevel_=0;overlayLastTime_=logoLastTime_=-1;
}
