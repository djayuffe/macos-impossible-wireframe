#include "Geometry.hpp"
#include "AdvancedGeometry.hpp"
#include "Timeline.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <set>
static void req(bool x,const char*m){if(!x){std::cerr<<"FAIL: "<<m<<"\n";std::exit(1);}}
static void mesh(const Mesh3&m,const char*n,bool requireEdges=true){std::string w;req(geo::validate(m,&w),n);req(!m.v.empty(),n);if(requireEdges)req(!m.e.empty(),n);}
int main(){
 auto t=geo::tesseract();req(t.v.size()==16&&t.e.size()==32&&t.faces.size()==24,"tesseract V/E/F");
 auto c16=geo::cell16();req(c16.v.size()==8&&c16.e.size()==24&&c16.faces.size()==32,"16-cell V/E/F");
 auto c24=geo::cell24();req(c24.v.size()==24&&c24.e.size()==96&&c24.faces.size()==96,"24-cell V/E/F");
 auto c600=geo::cell600();req(c600.v.size()==120&&c600.e.size()==720&&c600.faces.size()==1200,"600-cell V/E/F");
 auto c120=geo::cell120Vertices();std::string w;req(geo::validate(c120,&w),"120-cell valid");req(c120.v.size()==600&&c120.e.size()==1200,"120-cell V/E exact");
 mesh(geo::project4D(c600,.1f,.2f,3.4f,.3f,.4f),"4D projection"); mesh(geo::sliceFaces4D(c600,{.31f,.47f,.59f,.57f},0),"4D face slice");
 for(auto k:{geo::ImplicitKind::Gyroid,geo::ImplicitKind::SchwarzP,geo::ImplicitKind::SchwarzD,geo::ImplicitKind::Neovius,geo::ImplicitKind::IWP})mesh(geo::implicitSurface(k,12),"implicit extraction");
 mesh(geo::hopfFibres(8,24),"hopf");mesh(geo::boySurface(20,12),"boy");mesh(geo::superformula(24,12),"superformula");mesh(geo::cliffordTorus(20,10),"clifford");mesh(geo::hyperbolicBall(3,8),"hyperbolic");mesh(geo::lissajousKnot(100),"lissajous");mesh(geo::strangeAttractor(1000),"lorenz");mesh(geo::discoveredObject(42,24,12),"discovered");mesh(geo::quaternionJulia(14,7,.08f),"quaternion Julia");
 req(geo::noveltyScore(geo::discoveredObject(1,24,12))>0,"novelty finite positive");
 Timeline tl(120);auto s=tl.sample(.5);req(s.beatIndex==1&&std::fabs(s.beatPhase)<.001f,"timeline beat");req(s.barIndex==0&&s.barPhase>.24f&&s.barPhase<.26f,"timeline bar");
 std::cout<<"geometry_tests: PASS; exact 120-cell V="<<c120.v.size()<<" E="<<c120.e.size()<<"\n";
}
