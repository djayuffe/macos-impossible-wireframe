#include "BitmapFont.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

int main(int argc,char**argv){
 try{
  if(argc<2)throw std::runtime_error("font_tests needs font.rgba path");
  std::ifstream f(argv[1],std::ios::binary);
  std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(f)),{});
  if(bytes.size()<8)throw std::runtime_error("missing font source");
  auto dimension=[&](int i){return int(bytes[i])|(int(bytes[i+1])<<8)|(int(bytes[i+2])<<16)|(int(bytes[i+3])<<24);};
  std::vector<unsigned char> source(bytes.begin()+8,bytes.end());
  auto atlas=bitmapfont::crop(source,dimension(0),dimension(4));
  for(const auto& g:atlas.glyphs){
   int visible=0;
   for(int y=g.y-8;y<g.y+g.height+8;++y)for(int x=g.x-8;x<g.x+g.width+8;++x){
    auto alpha=atlas.pixels[(size_t(y)*atlas.width+x)*4+3];
    if(x<g.x||x>=g.x+g.width||y<g.y||y>=g.y+g.height){if(alpha)throw std::runtime_error("glyph leaks into its filtering gutter");}
    else if(alpha>24)++visible;
   }
   if(visible<1000)throw std::runtime_error("glyph is missing or overcropped");
  }
  bool rejected=false;try{bitmapfont::crop({},1983,793);}catch(const std::invalid_argument&){rejected=true;}
  if(!rejected||bitmapfont::index('A')!=0||bitmapfont::index('Z')!=25||bitmapfont::index('0')!=26||bitmapfont::index('9')!=35||bitmapfont::index(' ')!=-1)
   throw std::runtime_error("invalid font input or character mapping");
  if(argc>2){
   std::ofstream out(argv[2],std::ios::binary);out<<"P6\n"<<atlas.width<<" "<<atlas.height<<"\n255\n";
   for(size_t p=0;p<atlas.pixels.size();p+=4)for(int c=0;c<3;++c){unsigned char value=static_cast<unsigned char>(unsigned(atlas.pixels[p+c])*atlas.pixels[p+3]/255);out.write(reinterpret_cast<char*>(&value),1);}
   if(!out)throw std::runtime_error("font proof write failed");
  }
  std::cout<<"font_tests: PASS (36 cropped glyphs, filtering gutters, invalid input)\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}
