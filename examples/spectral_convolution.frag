uniform sampler2D MyTex;
uniform float dimen_1;
uniform float dimen_2;
uniform float direc;
uniform float resolution; //defines the number of filter coefficients
uniform float size;

uniform float kernel[20];

void main() {

	//--------------- defining the size and direction of the kernel---------------------------
	vec2 texSize = vec2(dimen_1, dimen_2);
	//vec2 texSize = vec2(float(gl_TextureMatrix[0][1] - gl_TextureMatrix[0][0]), float(gl_TextureMatrix[0][2] - gl_TextureMatrix[0][0]));
	vec2 uv = gl_FragCoord.xy / texSize;
	vec4 color = vec4(0.0);
	vec2 blur_dir = vec2(1.0, 0.0);
	if (direc == 0.0) {
		blur_dir = vec2(1.0, 0.0);
	}
	if (direc == 1.0) {
		blur_dir = vec2(0.0, 1.0);
	}

	//int stepsize = 129 - int(resolution);

	//-------------- iteration through the input array ---------------------------------------

	float kernel_sum = 0.0;
	float co = -10.;
	
	if (resolution <= 0.0) {
		gl_FragColor = texture2D(MyTex, uv);
	}
	else {
		for (int i = 0; i < 20; i+= 1) {
			
			co += float(i);
			kernel_sum += kernel[3];
			color += (texture2D(MyTex, uv + blur_dir * (size*co)/texSize)) * kernel[3];
			//color += (texture2D(MyTex, uv)) * kernel[i];
			
			}
		gl_FragColor = color/kernel_sum;
		}
}


