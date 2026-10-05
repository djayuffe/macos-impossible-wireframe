#include "BitmapFont.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace bitmapfont {
int index(char c){
 if(c>='A'&&c<='Z')return c-'A';
 if(c>='0'&&c<='9')return 26+c-'0';
 return -1;
}
Atlas crop(const std::vector<unsigned char>& rgba,int width,int height){
 if(width!=1983||height!=793||rgba.size()!=size_t(width)*height*4)
  throw std::invalid_argument("font sheet must be the supplied 1983x793 RGBA atlas");
 Atlas atlas;atlas.pixels.resize(size_t(atlas.width)*atlas.height*4);
 // Boundaries follow the actual lettering, not an assumed fixed grid. Some
 // italic glyphs overlap in X, so find a low-alpha seam separately for each Y.
 const std::vector<std::vector<int>> boundaries{
  {0,227,394,553,699,839,982,1127,1294,1380,1522,1691,1821,1983},
  {0,185,345,477,634,800,947,1085,1230,1370,1521,1694,1813,1983},
  {0,218,340,535,716,917,1103,1300,1485,1692,1983}
 };
 const int tops[]{0,183,368},bottoms[]{183,368,549};
 int glyphIndex=0;
 for(int row=0;row<3;++row){
  int top=tops[row],rows=bottoms[row]-top;
  const auto& b=boundaries[row];
  std::vector<std::vector<int>> seams(b.size(),std::vector<int>(rows));
  std::fill(seams.back().begin(),seams.back().end(),width);
  for(size_t edge=1;edge+1<b.size();++edge){
   int left=b[edge]-26,right=b[edge]+26,n=right-left+1;
   std::vector<float> prev(n),next(n);
   std::vector<int> parent(size_t(rows)*n);
   for(int y=0;y<rows;++y){
    for(int k=0;k<n;++k){
     int x=left+k;float best=y?std::numeric_limits<float>::max():0.f;int bestK=k;
     if(y)for(int pk=std::max(0,k-2);pk<=std::min(n-1,k+2);++pk){
      float cost=prev[pk]+.4f*std::abs(pk-k);if(cost<best){best=cost;bestK=pk;}
     }
     float a=rgba[(size_t(top+y)*width+x)*4+3]/255.f;
     next[k]=best+a*a*100.f+.004f*std::abs(x-b[edge]);
     parent[size_t(y)*n+k]=bestK;
    }
    prev.swap(next);
   }
   int k=int(std::min_element(prev.begin(),prev.end())-prev.begin());
   for(int y=rows-1;y>=0;--y){seams[edge][y]=left+k;k=parent[size_t(y)*n+k];}
  }
  for(size_t col=0;col+1<b.size();++col,++glyphIndex){
   int x0=width,x1=0,y0=rows,y1=0;
   for(int y=0;y<rows;++y)for(int x=seams[col][y];x<seams[col+1][y];++x){
    if(rgba[(size_t(top+y)*width+x)*4+3]>24){x0=std::min(x0,x);x1=std::max(x1,x+1);y0=std::min(y0,y);y1=std::max(y1,y+1);}
   }
   if(x1<=x0||y1<=y0)throw std::runtime_error("empty font glyph");
   x0=std::max(0,x0-2);x1=std::min(width,x1+2);y0=std::max(0,y0-2);y1=std::min(rows,y1+2);
   int gw=x1-x0,gh=y1-y0;
   if(gw>224||gh>192)throw std::runtime_error("font glyph exceeds padded cell");
   int dstX=(glyphIndex%13)*256+16,dstY=(glyphIndex/13)*224+16;
   atlas.glyphs[glyphIndex]={dstX,dstY,gw,gh,gw+14};
   for(int y=y0;y<y1;++y)for(int x=std::max(x0,seams[col][y]);x<std::min(x1,seams[col+1][y]);++x){
    size_t src=(size_t(top+y)*width+x)*4,dst=(size_t(dstY+y-y0)*atlas.width+dstX+x-x0)*4;
    if(rgba[src+3]>3)std::copy_n(rgba.data()+src,4,atlas.pixels.data()+dst);
   }
  }
 }
 return atlas;
}
}
