#define M_PI 3.1415926535897932384626433832795

#if defined(DIFFUSE_MAP) || defined(NORMAL_MAP) || defined(SPECULAR_MAP)
  #define TEXTURED
#endif

// @binding position
attribute vec3 aPosition;
// @binding normal
attribute vec3 aNormal;
#ifdef TANGENT
// @binding tangent
attribute vec3 aTangent;
// @binding bitangent
attribute vec3 aBitangent;
#endif
#ifdef TEXTURED
// @binding texture
attribute vec2 aTexture;
// @binding texture2
attribute vec2 aTexture2;
#endif
#ifdef COLORED
// @binding color
attribute vec3 aColor;
#endif
#ifdef SKINNED
// @binding indices
attribute vec4 aIndices;
// @binding weights
attribute vec4 aWeights;
#endif

varying vec4 vWorldPosition;
varying vec4 vWorldNormal;
varying vec4 vWorldTangent;
varying vec4 vWorldBitangent;
varying vec4 vViewPosition;
varying vec4 vViewNormal;
varying vec4 vColor;
varying vec4 vColor2;
varying vec2 vTexture;
varying vec4 vTexture2;

// @binding World
uniform mat4 uWorld;
// @binding View
uniform mat4 uView;
// @binding Projection
uniform mat4 uProjection;
// @binding ViewProjection
uniform mat4 uViewProjection;
// @binding WorldViewProjection
uniform mat4 uWorldViewProjection;

// @binding CameraDirection
uniform vec3 uCameraDirection;
// @binding CameraPosition
uniform vec3 uCameraPosition;

// @binding AmbientColor
// @widget   color
// @default  [0.2, 0.2, 0.2]
uniform vec3 uAmbientColor;

#ifdef HEMISPHERE
// @binding GroundColor
// @widget   color
// @default  [0.2, 0.2, 0.2]
uniform vec3 uGroundColor;

// @binding SkyDirection
// @widget   direction
// @default  [0.0, 1.0, 0.0]
uniform vec3 uSkyDirection;
#endif

// @binding DiffuseColor
// @widget   color
// @default  [1, 1, 1]
uniform vec3 uDiffuseColor;

// @binding SpecularColor
// @widget   color
// @default  [1, 1, 1]
uniform vec3 uSpecularColor;

// @binding Gamma
// @widget   range(0, 1)
// @default  2.2
uniform float uGamma;

// @binding Alpha
// @widget   range(0, 1)
// @default  1
uniform float uAlpha;

// @binding AlphaClip
// @widget   range(0, 1)
// @default  0
uniform float uAlphaClip;

// @binding SpecularPower
// @widget   range(0, 1)
// @default  0.5
uniform float uSpecularPower;

// @binding FogColor
// @widget   color
// @default  [1, 1, 1]
uniform float uFogColor;

// @binding FogStart
// @widget   number
// @default  [100]
uniform float uFogStart;

// @binding FogEnd
// @widget   number
// @default  [1000]
uniform float uFogEnd;

// @binding DiffuseMap
// @register 0
// @filter   LinearWrap
uniform sampler2D uDiffuseMap;

// @binding NormalMap
// @register 1
// @filter   LinearWrap
uniform sampler2D uNormalMap;

// @binding SpecularMap
// @register 2
// @filter   LinearWrap
uniform sampler2D uSpecularMap;

// @binding EmissionMap
// @register 3
// @filter   LinearWrap
uniform sampler2D uEmissionMap;

// @binding ReflectionMap
// @register 4
// @filter   LinearWrap
uniform sampler2D uReflectionMap;

// @binding LightMap
// @register 5
// @filter   LinearWrap
uniform sampler2D uLightMap;

// @binding OcclusionMap
// @register 6
// @filter   LinearWrap
uniform sampler2D uOcclusionMap;

struct LightParams {
  vec4 Position;  // xyz = position       , w is unused
  vec4 Direction; // xyz = direction      , w is unused
  vec4 Color;     // rgb = diffuse color  , a = specular amount
  vec4 Misc;      // xyz components are implementation dependent light attributes
                  //   w component is light type
                  //     0 = off
                  //     1 = directional,
                  //     2 = point
                  //     3 = spot
};

#ifndef NUM_LIGHTS
  #define NUM_LIGHTS 4
#endif
#ifndef NO_LIGHT
  #define NO_LIGHT 0
#endif
#ifndef DIRECTIONAL_LIGHT
  #define DIRECTIONAL_LIGHT 1
#endif
#ifndef POINT_LIGHT
  #define POINT_LIGHT 2
#endif
#ifndef SPOT_LIGHT
  #define SPOT_LIGHT 3
#endif

// @binding Lights
uniform LightParams uLights[NUM_LIGHTS];

struct ShadeParams {
  vec3 V; // Vector to eye
  vec3 L; // Vector to light
  vec3 I; // Light intensity
};

struct SurfaceParams {
  vec4 Normal;   // xyz = normal, w = depth
  vec4 Diffuse;  // rgb = albedo, a = alpha
  vec4 Specular; // rgb = specular color, a = specular power
  vec3 Emission; // rgb = emission color
};

#ifdef FRAGMENT_SHADER
void clipAlpha(float a) {
  #ifdef ALPHA_CLIP
  if ((a - uAlphaClip) <= 0.0) discard;
  #endif
}
#endif

void getDiffuseColor(out vec4 color) {
  #ifdef DIFFUSE_MAP
  color = texture2D(uDiffuseMap, vTexture.xy);
  color.a = color.a * uAlpha;
  #else
  color = vec4(uDiffuseColor, uAlpha);
  #ifdef COLORED
  color.rgb *= vColor.rgb;
  #endif
  #endif
}

#ifdef VERTEX_SHADER
void writePosition() {
  vWorldPosition = uWorld * vec4(aPosition, 1.0);
  gl_Position = uProjection * uView * vWorldPosition;
}

void writeNormal() {
  mat3 nrm = mat3(uWorld);
  vWorldNormal.xyz = nrm * aNormal;
  #if defined(TANGENT) && defined(TANGENT_TERRAIN)
  vWorldTangent.xyz = cross(nrm[0], vWorldNormal.xyz);
  vWorldBitangent.xyz = cross(vWorldTangent.xyz, vWorldNormal.xyz);
  #endif
  #if defined(TANGENT) && !defined(TANGENT_TERRAIN)
  vWorldTangent.xyz = nrm * aTangent;
  vWorldBitangent.xyz = nrm * aBitangent;
  #endif
}

void writeTexture() {
  #ifdef TEXTURED
  #ifdef TEXTURE_FLIP_Y
  vTexture.xy = vec2(aTexture.x, 1.0 - aTexture.y);
  #else
  vTexture.xy = aTexture.xy;
  #endif
  #endif
}

void writeColor() {
  #ifdef COLORED
  vColor.rgb = aColor.rgb;
  #endif
}
#endif

void getNormal(out vec3 bump){
  #if defined(TANGENT) && defined(NORMAL_MAP)
  bump = normalize(texture2D(uNormalMap, vTexture.xy).rgb * 2.0 - 1.0);
  bump = normalize(mat3(vWorldTangent.xyz, vWorldBitangent.xyz, vWorldNormal.xyz) * bump.xyz);
  #else
  bump = normalize(vWorldNormal.xyz);
  #endif
}

void getSpecularColor(out vec4 color) {
  #ifdef SPECULAR_MAP
    color = texture2D(uSpecularMap, vTexture.xy);
    color.a = exp2(uSpecularPower * 10.5);
  #else
    color = vec4(uSpecularColor, exp2(uSpecularPower * 10.5));
  #endif
}

void getAmbientColor(in vec3 normal, out vec3 color) {
  #ifdef HEMISPHERE
  color = mix(uAmbientColor.rgb, uGroundColor.rgb, 0.5 * (1.0 * dot(normal.xyz, uSkyDirection.xyz)));
  #else
  color = uAmbientColor.rgb;
  #endif
}

void getLight(in LightParams light, int type, in vec3 position, inout ShadeParams result) {
  #if DIRECTIONAL_LIGHT
  // directional light (constant attenuation)
  if (type == DIRECTIONAL_LIGHT)
  {
    result.L = normalize(-light.Direction.xyz);
    result.I = light.Color.rgb * light.Color.a;
    return;
  }
  #endif

  #if POINT_LIGHT
  // point light (radial linear attenuation)
  if (type == POINT_LIGHT)
  {
    float range = max(0.00001, light.Misc.x);
    vec3 toLight = light.Position.xyz - position;
    result.L = normalize(toLight);
    result.I = light.Color.rgb * light.Color.a * max(1.0 - min(1.0, length(toLight) / range), 1.0);
    return;
  }
  #endif

  #if SPOT_LIGHT
  // spot light (cone and linear attenuation)
  if (type == SPOT_LIGHT)
  {
    float range = max(0.00001, light.Misc.x);
    float cosOuter = light.Misc.y;
    float cosInner = light.Misc.z;
    vec3 toLight = light.Position.xyz - position;
    result.L = normalize(toLight);
    float spot = smoothstep(cosOuter, cosInner, dot(result.L, -light.Direction.xyz));
    result.I = light.Color.rgb * light.Color.a * (1.0 - min(1.0, length(toLight) / range)) * spot;
    return;
  }
  #endif

  result.I = vec3(0.0);
  return;
}


//
// SHADING
//

highp vec3 fresnelSchlick(vec3 R, float dotLH) {
  return R + (1.0 - R) * pow(1.0 - dotLH, 5.0);
  //return mix(R, min(60.0 * R, 1.0), pow(1.0 - dotLH, 4.0));
}

highp vec3 fresnelSchlickf90(in vec3 f0, float f90, float u) {
  return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

highp float disneyDiffuse(in float dotNV, in float dotNL, in float dotLH, float linearRoughness) {
  float energyBias = mix(0.0, 0.5, linearRoughness);
  float energyFactor = mix(1.0, 1.0 / 1.51, linearRoughness);
  float fd90 = energyBias + 2.0 * dotLH * dotLH * linearRoughness;
  vec3 f0 = vec3(1.0, 1.0, 1.0);
  float lightScatter = fresnelSchlickf90(f0, fd90, dotNL).r;
  float viewScatter = fresnelSchlickf90(f0, fd90, dotNV).r;
  return lightScatter * viewScatter * energyFactor;
}

highp vec3 shadeLambert(
  inout ShadeParams shade,
  inout SurfaceParams surface)
{
  vec3 N = surface.Normal.xyz;
  vec3 L = shade.L;
  vec3 I = shade.I;
  float dotNL = max(dot(N, L), 0.0);
  return surface.Diffuse.rgb * dotNL * I;
}

highp vec3 shadeCookTorrance(
  inout ShadeParams shade,
  inout SurfaceParams surface)
{
  vec3 V = shade.V;
  vec3 N = surface.Normal.xyz;
  vec3 L = shade.L;
  vec3 H = normalize(L + V);
  vec3 I = shade.I;

  float dotNL = max(dot(N, L), 0.0);
  float dotNH = max(dot(N, H), 0.0);
  float dotNV = max(dot(N, V), 0.0);
  float dotLH = max(dot(L, H), 0.0);

  // specular BRDF (Fr)
  float D = pow(dotNH, surface.Specular.a);
  vec3  F = fresnelSchlick(surface.Specular.rgb, dotLH);
  float G = min(1.0, 2.0 * dotNH * min(dotNV, dotNL) / dotLH);
  vec3  Fr = D * F * G / (4.0 * dotNV * dotNL);

  // diffuse BRDF (Fd)
  float Fd = dotNL;

  return (Fr * surface.Specular.rgb + Fd * surface.Diffuse.rgb) * I;
}

highp vec3 shadeSzirmay(
  inout ShadeParams shade,
  inout SurfaceParams surface)
{
  vec3 V = shade.V;
  vec3 N = surface.Normal.xyz;
  vec3 L = shade.L;
  vec3 H = normalize(V + L);
  vec3 I = shade.I;

  float dotNL = max(dot(N, L), 0.0);
  float dotNH = max(dot(N, H), 0.0);
  //float dotNV = max(dot(N, V), 0.0);
  float dotLH = max(dot(L, H), 0.0);

  // specular BRDF (Fr)
  float D = pow(dotNH, surface.Specular.a);
  vec3  F = fresnelSchlick(surface.Specular.rgb, dotLH);
  vec3  Fr = D * F / (4.0 * dotLH * dotLH);

  return (Fr * surface.Specular.rgb + surface.Diffuse.rgb) * dotNL * I;
}

highp vec3 shadeOptimized(
  inout ShadeParams shade,
  inout SurfaceParams surface)
{
  vec3 V = shade.V;
  vec3 N = surface.Normal.xyz;
  vec3 L = shade.L;
  vec3 H = normalize(V + L);
  vec3 I = shade.I;

  float dotNL = max(dot(N, L), 0.0);
  float dotNH = max(dot(N, H), 0.0);
  //float dotNV = max(dot(N, V), 0.0);
  float dotLH = max(dot(L, H), 0.0);

  // specular BRDF (Fr)
  float D = pow(dotNH, surface.Specular.a);
  float Fr = D / (4.0 * pow(dotLH, 3.0));

  return (Fr * surface.Specular.rgb + surface.Diffuse.rgb) * dotNL * I;
}

//
// Vertex shaders
//
#ifdef VERTEX_SHADER
void glibVertexShader() {
  writePosition();
  writeTexture();
  writeNormal();
  writeColor();
}

#ifndef VERTEX_SHADER_FUNCTION
#define VERTEX_SHADER_FUNCTION glibVertexShader
#endif
#endif

//
// Fragment shaders
//
#ifdef FRAGMENT_SHADER
void shadePixel(in SurfaceParams surface) {
  vec4 color = vec4(0);

  #ifdef GAMMA_CORRECTION
  surface.Diffuse.rgb = pow(surface.Diffuse.rgb, vec3(uGamma));
  surface.Specular.rgb = pow(surface.Specular.rgb, vec3(uGamma));
  //surface.Diffuse.rgb = surface.Diffuse.rgb * surface.Diffuse.rgb;
  //surface.Specular.rgb = surface.Specular.rgb * surface.Specular.rgb;
  #endif

  for (int i = 0; i < NUM_LIGHTS; i++)
  {
    LightParams light = uLights[i];
    int type = int(light.Misc.w);
    if (type <= 0) break; // stop on first light that is off

    ShadeParams shade;
    shade.V = normalize(uCameraPosition.xyz - vWorldPosition.xyz);
    getLight(light, type, vWorldPosition.xyz, shade);
    //color.rgb += shadeCookTorrance(shade, surface).rgb;
    //color.rgb += shadeSzirmay(shade, surface).rgb;
    color.rgb += shadeOptimized(shade, surface).rgb;
  }
  vec3 ambient;
  getAmbientColor(surface.Normal.xyz, ambient);
  color.rgb += ambient.rgb * surface.Diffuse.rgb;
  // color.rgb += surface.Emission.rgb;
  color.a = surface.Diffuse.a;

  // tone mapping
  color.rgb = 1.0 - exp(-color.rgb);

  // gamma conversion
  #ifdef GAMMA_CORRECTION
  color.rgb = pow(color.rgb, vec3(1.0/uGamma));
  #endif

  #ifdef DEBUG_TEXTURE
  color.rgb = surface.Diffuse.rgb;
  #endif

  #ifdef DEBUG_NORMAL
  color.rgb = surface.Normal.xyz;
  #endif

  gl_FragColor = color;
}

void glibFragmentShader() {
  SurfaceParams surface;
  //callSurface(surface);
  getDiffuseColor(surface.Diffuse);
  #ifdef ALPHA_CLIP
  clipAlpha(surface.Diffuse.a);
  #endif
  getSpecularColor(surface.Specular);
  getNormal(surface.Normal.xyz);
  shadePixel(surface);
}

#ifndef FRAGMENT_SHADER_FUNCTION
#define FRAGMENT_SHADER_FUNCTION glibFragmentShader
#endif
#endif
