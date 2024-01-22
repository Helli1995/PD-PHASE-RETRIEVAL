uniform sampler2D MyTex;
uniform float dimen_1;
uniform float dimen_2;

uniform float NOISE_A,
NOISE_S;
float PI2 = 6.28319;

// warper / profile functions ----------------
vec2 tiler(vec2 U, vec2 T) {
	return fract(U*T);
}
vec2 scale(vec2 U, vec2 S) {
	return clamp( .5+(U-.5)/S, 0., 1. );
}
vec2 noise2(vec2 p);
vec2 displace(vec2 U, float amp, float scale) {
	return  U += amp/scale* noise2(U*scale);
}
// --------------------------------------------------------

#define hash22(p)  fract( 18.5453 * sin( (p) * mat2(127.1,311.7,269.5,183.3)) )
vec2 noise2(vec2 p) {
	vec2 i = floor(p);
	vec2 f = fract(p); f = f*f*(3.-2.*f); // smoothstep
	vec2 v= mix( mix(hash22(i+vec2(0,0)),hash22(i+vec2(1,0)),f.x),
				  mix(hash22(i+vec2(0,1)),hash22(i+vec2(1,1)),f.x), f.y);
	return 2.*v-1.;
}

vec2 texSize = vec2(dimen_1, dimen_2);
vec2 U = gl_FragCoord.xy / texSize;

void main()
{

	// top right: displacement
	//U = 2./U;
	U = displace(U,NOISE_A,NOISE_S);
	
	gl_FragColor = texture2D(MyTex, U);
}
