uniform sampler2D MyTex;
uniform float dimen_1;
uniform float dimen_2;
uniform float th;
uniform float size_e;
//uniform float strength;
/**
 * Edge Detection: 834144373's https://www.shadertoy.com/view/MdGGRt
 * Bilateral Filter: https://www.shadertoy.com/view/4dfGDH
 */

float sigmoid(float a, float f) {
	return 1.0 / (1.0 + exp(-f * a));
}


void main()
{
	vec2 texSize = vec2(dimen_1, dimen_2);
	vec2 uv = gl_FragCoord.xy/texSize;
	
	float edgeStrength = length(fwidth(texture2D(MyTex, uv)));
	edgeStrength = sigmoid(edgeStrength - th, size_e);
	gl_FragColor = vec4(vec3(edgeStrength), 1.0);
}
