uniform sampler2D MyTex;
uniform float X;
uniform float Y;
uniform float num;
uniform float dimen_1;
uniform float dimen_2;
uniform float wet;
uniform float x_off;
uniform float y_off;

vec2 texSize = vec2(dimen_1, dimen_2);
vec2 coord = gl_FragCoord.xy / texSize;
float dry = 1. - wet;
void main ()
{
	
	float X = X/(dimen_1);
	float Y = Y/(dimen_2);
	
	vec4 color_dry = texture2D(MyTex, coord)*dry;
	vec4 color_wet = vec4(0.);
	if ((num <= 1.0) || (dry < 0.0)) {
		gl_FragColor = texture2D(MyTex, coord);
	}
	else {
		
		for (float i = 1.0; i <= num; i+= 1.) {
			vec2 reflector = vec2(i*Y, i*X);
			vec2 mirror = reflect(coord - vec2(x_off,y_off), reflector);
			color_wet += texture2D(MyTex, mirror+(vec2(x_off,y_off)));
		}
		color_wet = (color_wet/num) * wet;
		gl_FragColor = color_dry + color_wet;
	}
}

