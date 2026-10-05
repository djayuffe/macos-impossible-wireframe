#version 410 core
in vec2 vUv;
out vec4 o;
uniform sampler2D uTex;
uniform sampler2D uScene;
uniform float uAlpha;
uniform vec3 uTint;
uniform float uTime;
uniform float uFx;
uniform vec2 uResolution;
uniform bool uScroller;
float bolt(vec2 p,float seed) {
  float y=.16*sin(p.x*(13.+seed*4.)+uTime*(4.5+seed))+.07*sin(p.x*(37.+seed*9.)-uTime*7.)+.12*sin(seed*8.);
  return exp(-abs(p.y-y)*(150.+uFx*80.))*(1.-smoothstep(.12,1.1,abs(p.x)));
}
void main() {
  vec2 screen=gl_FragCoord.xy/uResolution;
  vec2 uv=vUv;
  if(uv.y<0. || uv.y>1.) discard;
  vec2 texel=1./vec2(textureSize(uTex,0));
  vec2 ca=vec2(uScroller?0.:texel.x*1.2*uFx*(.5+.5*sin(uTime*5.)),0.);
  vec4 c=texture(uTex,uv);
  c.r=mix(c.r,texture(uTex,uv+ca).r,.35*uFx);
  c.b=mix(c.b,texture(uTex,uv-ca).b,.35*uFx);
  float around=max(max(texture(uTex,uv+vec2(texel.x*2.,0.)).a,texture(uTex,uv-vec2(texel.x*2.,0.)).a),max(texture(uTex,uv+vec2(0.,texel.y*2.)).a,texture(uTex,uv-vec2(0.,texel.y*2.)).a));
  float halo=max(around-c.a,0.)*(1.-c.a);
  float energy=clamp(dot(texture(uScene,screen).rgb,vec3(.22,.38,.68))*.18,0.,1.);
  float sweep=1.-smoothstep(0.,.055,abs(fract(screen.x+uTime*.16)-.5));
  float arc=bolt(screen*2.-1.,1.)+bolt((screen.yx*2.-1.)*vec2(1.,-1.),2.);
  float lightning=halo*arc*uFx*(.75+energy*2.2);
  float alpha=clamp(c.a+lightning*.8+halo*energy*.32,0.,1.)*uAlpha;
  vec3 glow=c.rgb*uTint*(1.+.3*sweep*uFx+energy*.35);
  vec3 hot=vec3(1.,.18,.06)*pow(max(c.r-c.b,0.),1.7)*.3*uFx;
  vec3 electric=mix(vec3(.02,.45,1.3),vec3(1.2,.04,.01),step(c.b,c.r));
  float scan=.97+.03*sin(gl_FragCoord.y*3.14159265+uTime*4.);
  o=vec4((glow+hot+electric*lightning*1.8)*scan,alpha);
}
