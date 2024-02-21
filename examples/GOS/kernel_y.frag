#version 120
uniform float myKernel[512];
uniform float size;
uniform float array_len;
uniform sampler2D MyTex;
uniform float dimen_1;
uniform float dimen_2;
uniform float direc;
uniform float dist;
//uniform float len; //length of the filter kernel
int max_size;
vec2 texSize = vec2(dimen_1, dimen_2);
vec2 uv = gl_FragCoord.xy / texSize;
float coord = -size;
//-------------- calculating array parameters ----------------------------------------------------
vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
float kernel_sum = 0.;
float ratio = (array_len - 1.) / (2. * size + 1.);

void main() {
	//--------------- defining the size and direction of the kernel---------------------------
	vec2 blur_dir;
	if (direc == 0.0) {
		blur_dir = vec2(1.0, 0.0);
		max_size = int(dimen_2);
	}
	
	if (direc == 1.0) {
		blur_dir = vec2(0.0, 1.0);
		max_size = int(dimen_1);
	}
	
	//-------------- iteration through the input array ---------------------------------------
	if ((size <= 0.) || (array_len <= 0.)) {
		gl_FragColor = texture2D(MyTex, uv);
	}
	else {
		for (int i = 0; i < (2 * int(size)) + 1; i+= 1) {
			
			//---resize the array values to the specified size-----------------
			int low = int(floor(ratio * float(i)));
			int high = int(ceil(ratio * float(i)));
			float weight = ratio * float(i) - float(low);

			float a = myKernel[low];
			float b = myKernel[high];

			float c = a * (1. - weight) + b * weight;
			c = clamp(c, 0., 1.);
			
			//-----------------------------------------------------------------
			
			kernel_sum += c;
			color += (texture2D(MyTex, uv + blur_dir  * ((coord * dist)/texSize))) * c;
			coord += 1.;
		}
		gl_FragColor = clamp((color/kernel_sum), 0., 1.);
	}
}









