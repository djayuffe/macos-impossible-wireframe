#version 410 core
in vec2 uv;
out vec4 FragColor;
uniform sampler2D uSource;
uniform vec2 uDirection;
uniform bool uExtract;
vec3 sampleColor(vec2 p) {
  vec3 c=texture(uSource,p).rgb;
  if(uExtract) {
    float brightness=max(c.r,max(c.g,c.b));
    c*=max(brightness-1.,0.)/max(brightness,.0001);
  }
  return c;
}
void main() {
  // Separable Gaussian with linear-filter paired taps, half-resolution HDR.
  vec3 c=sampleColor(uv)*.2270270270;
  c+=(sampleColor(uv+uDirection*1.3846153846)+sampleColor(uv-uDirection*1.3846153846))*.3162162162;
  c+=(sampleColor(uv+uDirection*3.2307692308)+sampleColor(uv-uDirection*3.2307692308))*.0702702703;
  FragColor=vec4(c,1.);
}
