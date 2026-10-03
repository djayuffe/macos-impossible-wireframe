#include "Scene.hpp"
#include "AdvancedGeometry.hpp"
#include <algorithm>
#include <array>
#include <cmath>
namespace {
constexpr std::array<SceneInfo,16> S{{
 {0,"600-cell projection",true,60,"exact 600-cell + 4D projection"},{1,"600-cell slice",true,30,"exact face/hyperplane intersection"},
 {2,"Gyroid",true,6,"numerical implicit"},{3,"Schwarz P",true,6,"numerical implicit"},{4,"Schwarz D",true,6,"numerical implicit"},{5,"Neovius",true,6,"numerical implicit"},
 {6,"Hopf fibres",false,1,"parametric S3 stereographic projection"},{7,"Boy surface",false,1,"analytic immersion"},{8,"Superformula",true,15,"parametric"},
 {9,"120-cell projection",true,60,"exact dual incidence + 4D projection"},{10,"Clifford torus",false,1,"parametric S3 stereographic projection"},
 {11,"Hyperbolic ball",false,1,"artistic Poincare-ball visualization"},{12,"Quaternion Julia slice",true,3,"numerical escape-boundary lattice"},
 {13,"Lissajous knot",true,15,"parametric curve"},{14,"Lorenz attractor",false,1,"numerical trajectory"},{15,"Discovered object",true,.125,"deterministic seeded harmonic surface"}
}};
}
SceneSystem::SceneSystem():c600_(geo::cell600()),c120_(geo::cell120Vertices()){}
const SceneInfo& SceneSystem::info(int id)const{return S[std::clamp(id,0,int(S.size()-1))];}
int SceneSystem::count()const{return int(S.size());}
const Mesh3& SceneSystem::mesh(int id,double seconds,uint64_t seed){
 id=std::clamp(id,0,count()-1);auto& si=info(id);uint64_t tick=si.dynamic?uint64_t(std::floor(std::max(0.0,seconds)*si.updateHz)):0;
 if(id==cachedId_&&tick==cachedTick_) return cache_;
 float t=float(seconds);
 switch(id){case 0:cache_=geo::project4D(c600_,t*.17f,t*.29f,3.4f,t*.09f,t*.13f);break;case 1:cache_=geo::sliceFaces4D(c600_,{.31f,.47f,.59f,.57f},std::sin(t*.31f)*1.45f);break;
 case 2:cache_=geo::implicitSurface(geo::ImplicitKind::Gyroid,25,std::sin(t*.22f)*.25f,t*.08f);break;case 3:cache_=geo::implicitSurface(geo::ImplicitKind::SchwarzP,25,0,t*.1f);break;case 4:cache_=geo::implicitSurface(geo::ImplicitKind::SchwarzD,24,0,t*.08f);break;case 5:cache_=geo::implicitSurface(geo::ImplicitKind::Neovius,24,0,t*.07f);break;
 case 6:cache_=geo::hopfFibres(42,112,2.3f);break;case 7:cache_=geo::boySurface(84,44,.95f);break;case 8:cache_=geo::superformula(96,48,5.f+2.f*std::sin(t*.13f),3.f+2.f*std::sin(t*.17f),.38f,1.6f,1.6f);break;
 case 9:cache_=geo::project4D(c120_,t*.09f,t*.13f,8.5f,t*.07f,t*.11f);break;case 10:cache_=geo::cliffordTorus(56,30,2.2f);break;case 11:cache_=geo::hyperbolicBall(7,20,.9f);break;
 case 12:cache_=geo::quaternionJulia(24,8,.055f,{-.2f,.7f,.05f*std::sin(t*.1f),0});break;case 13:cache_=geo::lissajousKnot(1800,3,4,7,.4f+t*.03f);break;case 14:cache_=geo::strangeAttractor(14000,.004f);break;default:cache_=geo::discoveredObject(seed+tick,88,44);break;}
 cachedId_=id;cachedTick_=tick;return cache_;
}
