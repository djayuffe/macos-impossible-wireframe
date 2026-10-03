#include "AdvancedGeometry.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <random>
#include <set>
namespace { constexpr float PI=3.14159265358979323846f;
float val(geo::ImplicitKind k,float x,float y,float z,float p){x+=p;y-=p*.71f;z+=p*.37f;switch(k){case geo::ImplicitKind::Gyroid:return std::sin(x)*std::cos(y)+std::sin(y)*std::cos(z)+std::sin(z)*std::cos(x);case geo::ImplicitKind::SchwarzP:return std::cos(x)+std::cos(y)+std::cos(z);case geo::ImplicitKind::SchwarzD:return std::sin(x)*std::sin(y)*std::sin(z)+std::sin(x)*std::cos(y)*std::cos(z)+std::cos(x)*std::sin(y)*std::cos(z)+std::cos(x)*std::cos(y)*std::sin(z);case geo::ImplicitKind::Neovius:return 3*(std::cos(x)+std::cos(y)+std::cos(z))+4*std::cos(x)*std::cos(y)*std::cos(z);case geo::ImplicitKind::IWP:return 2*(std::cos(x)*std::cos(y)+std::cos(y)*std::cos(z)+std::cos(z)*std::cos(x))-(std::cos(2*x)+std::cos(2*y)+std::cos(2*z));}return 0;}
uint32_t add(Mesh3&m,V3 p,float e=2e-4f){for(uint32_t i=0;i<m.v.size();++i){auto q=m.v[i];float x=p.x-q.x,y=p.y-q.y,z=p.z-q.z;if(x*x+y*y+z*z<e*e)return i;}m.v.push_back(p);return uint32_t(m.v.size()-1);}
void dedup(Mesh3&m){std::set<std::pair<uint32_t,uint32_t>>s;std::vector<Edge>o;for(auto e:m.e){auto p=std::minmax(e.a,e.b);if(p.first!=p.second&&s.insert(p).second)o.push_back({p.first,p.second});}m.e.swap(o);}
V4 qm(V4 a,V4 b){return {a.x*b.x-a.y*b.y-a.z*b.z-a.w*b.w,a.x*b.y+a.y*b.x+a.z*b.w-a.w*b.z,a.x*b.z-a.y*b.w+a.z*b.x+a.w*b.y,a.x*b.w+a.y*b.z-a.z*b.y+a.w*b.x};}
float qn(V4 a){return std::sqrt(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w);}
}
namespace geo {
std::string implicitName(ImplicitKind k){switch(k){case ImplicitKind::Gyroid:return"Gyroid";case ImplicitKind::SchwarzP:return"Schwarz P";case ImplicitKind::SchwarzD:return"Schwarz D";case ImplicitKind::Neovius:return"Neovius";case ImplicitKind::IWP:return"I-WP";}return"Implicit";}
Mesh3 implicitSurface(ImplicitKind kind,unsigned n,float iso,float phase){n=std::clamp(n,5u,64u);Mesh3 m;const int T[6][4]={{0,5,1,6},{0,1,2,6},{0,2,3,6},{0,3,7,6},{0,7,4,6},{0,4,5,6}},C[8][3]={{0,0,0},{1,0,0},{1,1,0},{0,1,0},{0,0,1},{1,0,1},{1,1,1},{0,1,1}};for(unsigned z=0;z<n-1;z++)for(unsigned y=0;y<n-1;y++)for(unsigned x=0;x<n-1;x++){V3 P[8];float D[8];for(int c=0;c<8;c++){float X=-PI+2*PI*(x+C[c][0])/(n-1),Y=-PI+2*PI*(y+C[c][1])/(n-1),Z=-PI+2*PI*(z+C[c][2])/(n-1);P[c]={X/PI,Y/PI,Z/PI};D[c]=val(kind,X,Y,Z,phase)-iso;}for(auto&t:T){std::vector<V3>h;for(int a=0;a<4;a++)for(int b=a+1;b<4;b++){int i=t[a],j=t[b];if((D[i]<0)==(D[j]<0))continue;float u=D[i]/(D[i]-D[j]);h.push_back({P[i].x+u*(P[j].x-P[i].x),P[i].y+u*(P[j].y-P[i].y),P[i].z+u*(P[j].z-P[i].z)});}if(h.size()>=3){std::vector<uint32_t>id;for(auto p:h)id.push_back(add(m,p));for(size_t i=0;i<id.size();i++)m.e.push_back({id[i],id[(i+1)%id.size()]});}}}dedup(m);return m;}
Polytope4 cell120Vertices(){
 // Exact combinatorial dual construction: each tetrahedral cell of the canonical
 // 600-cell becomes one 120-cell vertex; cells sharing a triangular face become edges.
 Polytope4 src=cell600(), out; const size_t N=src.v.size();
 std::vector<std::set<uint32_t>> adj(N); for(auto e:src.e){adj[e.a].insert(e.b);adj[e.b].insert(e.a);}
 std::vector<std::array<uint32_t,4>> cells;
 for(uint32_t a=0;a<N;a++) for(uint32_t b:adj[a]) if(a<b) for(uint32_t c:adj[a]) if(b<c&&adj[b].count(c))
   for(uint32_t d:adj[a]) if(c<d&&adj[b].count(d)&&adj[c].count(d)) cells.push_back({a,b,c,d});
 for(auto C:cells){V4 q{};for(auto i:C){q.x+=src.v[i].x;q.y+=src.v[i].y;q.z+=src.v[i].z;q.w+=src.v[i].w;}q.x/=4;q.y/=4;q.z/=4;q.w/=4;out.v.push_back(q);}
 std::map<std::array<uint32_t,3>,uint32_t> owner;
 for(uint32_t ci=0;ci<cells.size();++ci) for(int omit=0;omit<4;omit++){std::array<uint32_t,3> f{};int k=0;for(int j=0;j<4;j++)if(j!=omit)f[k++]=cells[ci][j];std::sort(f.begin(),f.end());auto it=owner.find(f);if(it==owner.end())owner[f]=ci;else out.e.push_back({it->second,ci});}
 return out;
}
Mesh3 quaternionJulia(unsigned n,unsigned it,float th,V4 c){
 n=std::clamp(n,8u,48u); th=std::max(th,.005f); Mesh3 m;
 // Sample escape metric on a 3-D slice (w=0), retain near-boundary lattice
 // points, then connect only axis-neighbour boundary samples. This avoids the
 // false scan-order polyline used by v2.
 const size_t NN=size_t(n)*n*n; std::vector<float> metric(NN); std::vector<uint8_t> boundary(NN,0);
 auto idx=[&](unsigned x,unsigned y,unsigned z){return (size_t(z)*n+y)*n+x;};
 auto eval=[&](float x,float y,float z){V4 q{x,y,z,0}; unsigned k=0; for(;k<it;k++){q=qm(q,q);q={q.x+c.x,q.y+c.y,q.z+c.z,q.w+c.w};if(qn(q)>4.f)break;} return float(k)+std::min(1.f,qn(q)/4.f);};
 for(unsigned z=0;z<n;z++)for(unsigned y=0;y<n;y++)for(unsigned x=0;x<n;x++){
  float X=-1.55f+3.1f*x/(n-1),Y=-1.55f+3.1f*y/(n-1),Z=-1.55f+3.1f*z/(n-1); metric[idx(x,y,z)]=eval(X,Y,Z);
 }
 const int D[6][3]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
 for(unsigned z=0;z<n;z++)for(unsigned y=0;y<n;y++)for(unsigned x=0;x<n;x++){
  float v=metric[idx(x,y,z)]; bool edge=false; for(auto&d:D){int X=int(x)+d[0],Y=int(y)+d[1],Z=int(z)+d[2];if(X<0||Y<0||Z<0||X>=int(n)||Y>=int(n)||Z>=int(n))continue;float dv=std::fabs(v-metric[idx(X,Y,Z)]);if(dv>std::max(.35f,th*8.f)){edge=true;break;}}
  boundary[idx(x,y,z)]=edge;
 }
 std::vector<int32_t> map(NN,-1);
 for(unsigned z=0;z<n;z++)for(unsigned y=0;y<n;y++)for(unsigned x=0;x<n;x++)if(boundary[idx(x,y,z)]){map[idx(x,y,z)]=(int32_t)m.v.size();m.v.push_back({-1.55f+3.1f*x/(n-1),-1.55f+3.1f*y/(n-1),-1.55f+3.1f*z/(n-1)});}
 const int P[3][3]={{1,0,0},{0,1,0},{0,0,1}};
 for(unsigned z=0;z<n;z++)for(unsigned y=0;y<n;y++)for(unsigned x=0;x<n;x++){int32_t a=map[idx(x,y,z)];if(a<0)continue;for(auto&d:P){unsigned X=x+d[0],Y=y+d[1],Z=z+d[2];if(X>=n||Y>=n||Z>=n)continue;int32_t b=map[idx(X,Y,Z)];if(b>=0)m.e.push_back({uint32_t(a),uint32_t(b)});}}
 return m;
}
Mesh3 hyperbolicBall(unsigned shells,unsigned spokes,float curv){Mesh3 m;shells=std::max(2u,shells);spokes=std::max(6u,spokes);m.v.push_back({0,0,0});for(unsigned s=1;s<=shells;s++){float r=std::tanh(curv*s/shells*1.55f);for(unsigned a=0;a<spokes;a++){float u=2*PI*a/spokes;for(unsigned b=1;b<spokes/2;b++){float v=PI*b/(spokes/2);m.v.push_back({r*std::sin(v)*std::cos(u),r*std::cos(v),r*std::sin(v)*std::sin(u)});}}}for(uint32_t i=1;i<m.v.size();i++){float best=1e9;uint32_t bi=0;for(uint32_t j=0;j<i;j++){auto a=m.v[i],b=m.v[j];float d=(a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y)+(a.z-b.z)*(a.z-b.z);if(d<best){best=d;bi=j;}}m.e.push_back({i,bi});}return m;}
Mesh3 cliffordTorus(unsigned U,unsigned V,float pw){Mesh3 m;for(unsigned i=0;i<U;i++)for(unsigned j=0;j<V;j++){float u=2*PI*i/U,v=2*PI*j/V;V4 q{std::cos(u)/std::sqrt(2.f),std::sin(u)/std::sqrt(2.f),std::cos(v)/std::sqrt(2.f),std::sin(v)/std::sqrt(2.f)};float k=pw/(pw-q.w);m.v.push_back({q.x*k,q.y*k,q.z*k});m.e.push_back({i*V+j,i*V+(j+1)%V});m.e.push_back({i*V+j,((i+1)%U)*V+j});}return m;}
Mesh3 lissajousKnot(unsigned N,int p,int q,int r,float ph){Mesh3 m;N=std::max(64u,N);for(unsigned i=0;i<N;i++){float t=2*PI*i/N;m.v.push_back({std::sin(p*t+ph),std::sin(q*t),std::sin(r*t+ph*.37f)});m.e.push_back({i,(i+1)%N});}return m;}
Mesh3 strangeAttractor(unsigned N,float dt){Mesh3 m;float x=.1f,y=0,z=0;for(unsigned i=0;i<N;i++){float dx=10*(y-x),dy=x*(28-z)-y,dz=x*y-(8.f/3)*z;x+=dx*dt;y+=dy*dt;z+=dz*dt;if(i>100){m.v.push_back({x*.045f,y*.045f,(z-25)*.045f});if(m.v.size()>1)m.e.push_back({uint32_t(m.v.size()-2),uint32_t(m.v.size()-1)});}}return m;}
Mesh3 discoveredObject(uint64_t seed,unsigned U,unsigned V){std::mt19937_64 g(seed);std::uniform_real_distribution<float>d(.2f,2.8f);float a=d(g),b=d(g),c=d(g),m=2+int(g()%11),n=2+int(g()%9);Mesh3 out;for(unsigned i=0;i<U;i++)for(unsigned j=0;j<V;j++){float u=2*PI*i/U,v=-PI/2+PI*j/(V-1);float rr=1+.22f*std::sin(m*u+a)+.17f*std::cos(n*v+b)+.11f*std::sin((m+n)*u*v+c);out.v.push_back({rr*std::cos(v)*std::cos(u),rr*std::sin(v),rr*std::cos(v)*std::sin(u)});uint32_t k=i*V+j;if(i+1<U)out.e.push_back({k,(i+1)*V+j});else out.e.push_back({k,j});if(j+1<V)out.e.push_back({k,k+1});}return out;}
float noveltyScore(const Mesh3&m){auto s=stats(m);if(!s.vertices||!s.edges||!s.finite)return 0;float density=float(s.edges)/s.vertices;return std::log1p(float(s.vertices))*.35f+std::min(4.f,density)*.5f+std::min(3.f,s.radius)*.2f;}
}
