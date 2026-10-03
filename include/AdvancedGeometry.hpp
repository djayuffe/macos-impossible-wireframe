#pragma once
#include "Geometry.hpp"
#include <cstdint>
#include <string>
namespace geo {
enum class ImplicitKind { Gyroid, SchwarzP, SchwarzD, Neovius, IWP };
Mesh3 implicitSurface(ImplicitKind kind,unsigned n=28,float iso=0.f,float phase=0.f);
Polytope4 cell120Vertices();
Mesh3 quaternionJulia(unsigned n=30,unsigned iterations=9,float threshold=.035f,V4 c={-.2f,.7f,0.f,0.f});
Mesh3 hyperbolicBall(unsigned shells=6,unsigned spokes=18,float curvature=.82f);
Mesh3 cliffordTorus(unsigned u=48,unsigned v=24,float projectionW=2.4f);
Mesh3 lissajousKnot(unsigned samples=1200,int p=3,int q=4,int r=5,float phase=.3f);
Mesh3 strangeAttractor(unsigned samples=18000,float dt=.004f);
Mesh3 discoveredObject(uint64_t seed,unsigned u=72,unsigned v=36);
float noveltyScore(const Mesh3& m);
std::string implicitName(ImplicitKind k);
}
