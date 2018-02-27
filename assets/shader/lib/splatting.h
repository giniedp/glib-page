// @register 3
// @filter   LinearWrap
uniform sampler2D textureSplat;

// @register 4
// @filter   LinearWrap
uniform sampler2D textureDiffuse;

// @register 5
// @filter   LinearWrap
uniform sampler2D textureDiffuseR;

// @register 6
// @filter   LinearWrap
uniform sampler2D textureDiffuseG;

// @register 7
// @filter   LinearWrap
uniform sampler2D textureDiffuseB;

// @register 8
// @filter   LinearWrap
uniform sampler2D textureDiffuseA;

// @register 9
// @filter   LinearWrap
uniform sampler2D textureDiffuseSlope;


// @register 10
// @filter   LinearWrap
uniform sampler2D textureNormal;

// @register 11
// @filter   LinearWrap
uniform sampler2D textureNormalR;

// @register 12
// @filter   LinearWrap
uniform sampler2D textureNormalG;

// @register 13
// @filter   LinearWrap
uniform sampler2D textureNormalB;

// @register 14
// @filter   LinearWrap
uniform sampler2D textureNormalA;

// @register 15
// @filter   LinearWrap
uniform sampler2D textureNormalSlope;

// @register 16
// @filter   LinearWrap
uniform sampler2D textureTint;

// @binding Brightness
// @default 1.25
uniform float uBrightness;

// @binding Saturation
// @default 0.5
uniform float uSaturation;

// @binding Pertubation
// @default 0.25
uniform float uPertubation;

// @binding Tiling
// @default 64.0
uniform float uTiling;

//-----------------------------------------------------------------------------
// Calculate the transition factor to blend between clipmaps
//-----------------------------------------------------------------------------
// float BlendClipmaps(vec2 position) {
//   float fac =  N * G;
//   float fac2 = fac / 12.0;
//   vec2 dist = abs(position - ClipCenter.xz);
//   vec2 term = dist - ((fac - G) / 2 - fac2 - 2 * G);
//   term /= fac2;
//
// 	float result = max(term.x, term.y);
// 	return clamp(result, 0, 1);
// }

void adjustSaturation(inout vec4 color, float saturation)
{
    // The constants 0.3, 0.59, and 0.11 are chosen because the
    // human eye is more sensitive to green light, and less to blue.
    float grey = dot(color.xyz, vec3(0.3, 0.59, 0.11));
    color.rgb = mix(vec3(grey), color.rgb, saturation);
}

float blendSlope(in float slope, in vec2 uv){
  float blend = texture2D(textureDiffuseSlope, uv).r;

  if(slope < 0.5){
    blend = 2.0 * slope * blend;
  } else {
    blend = 1.0 - 2.0 * (1.0 - slope) * (1.0 - blend);
  }
  return clamp((blend - 0.5) * 5.0 + 0.5, 0.0, 1.0);
}

vec4 splatColor(in vec4 uv, in vec4 splat, in float slope){
	vec4 tempColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec2 uv0 = uv.xy;
	vec2 uv1 = uv.xy * vec2(uPertubation, uPertubation);
	float brightness = uBrightness;//1.5;
	float saturatuion = uSaturation;

	tempColor = texture2D(textureDiffuse, uv0) * brightness;
	adjustSaturation(tempColor, saturatuion);
	tempColor *= texture2D(textureDiffuse, uv1) * brightness;

	vec4 color = tempColor;

	tempColor = texture2D(textureDiffuseR, uv0) * brightness;
	adjustSaturation(tempColor, saturatuion);
	tempColor *= texture2D(textureDiffuseR, uv1) * brightness;
	color = mix(color.rgba, tempColor, splat.r);

	tempColor = texture2D(textureDiffuseG, uv0) * brightness;
	adjustSaturation(tempColor, saturatuion);
	tempColor *= texture2D(textureDiffuseG, uv1) * brightness;
	color = mix(color.rgba, tempColor, splat.g);

	tempColor = texture2D(textureDiffuseB, uv0) * brightness;
	adjustSaturation(tempColor, saturatuion);
	tempColor *= texture2D(textureDiffuseB, uv1) * brightness;
	color = mix(color.rgba, tempColor, splat.b);

  #ifdef SPLAT_A
	tempColor = texture2D(textureDiffuseA, uv0) * brightness;
	adjustSaturation(tempColor, saturatuion);
	tempColor *= texture2D(textureDiffuseA, uv1) * brightness;
	color = mix(color.rgba, tempColor, splat.a);
  #endif

  tempColor = texture2D(textureDiffuseSlope, uv.xy) * brightness;
  // adjustSaturation(tempColor, saturatuion);
	// tempColor *= texture2D(textureDiffuseSlope, uv1) * brightness;
	color = mix(color.rgba, tempColor, slope);

  #ifdef TINT_LAYER
	color *= texture2D(textureTint, uv.zw);
  #endif

	return color;
}

vec3 splatNormal(in vec4 uv, in vec4 splat, in float slope){
	vec4 normal = texture2D(textureNormal, uv.xy);
	normal = mix(normal, texture2D(textureNormalR, uv.xy), splat.r);
	normal = mix(normal, texture2D(textureNormalG, uv.xy), splat.g);
	normal = mix(normal, texture2D(textureNormalB, uv.xy), splat.b);
  #ifdef SPLAT_A
	normal = mix(normal, texture2D(textureNormalA, uv.xy), splat.a);
  #endif
	normal = mix(normal, texture2D(textureNormalSlope, uv.xy), slope);
	return normalize(normal.xzy * 2.0 - 1.0);
}
