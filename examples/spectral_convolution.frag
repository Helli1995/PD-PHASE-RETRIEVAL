uniform sampler2D MyTex;
uniform float dimen_1;
uniform float dimen_2;
uniform float direc;
uniform float resolution; //defines the number of points used for convolution
//uniform float size;

uniform float kernel[512];

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
	
	int stepsize = 129 - int(resolution);
    
//-------------- iteration through the input array ---------------------------------------

    float kernel_sum = 0.0;
	float coord = - 256.0;
	
	if (resolution <= 0.0) {
		gl_FragColor = texture2D(MyTex, uv);
	}
 
    for (int i = (0); i < 512; i+= stepsize) {
		
			coord += float(i);
            kernel_sum += kernel[i];
            color += (texture2D(MyTex, uv + blur_dir * (coord/texSize))) * kernel[i];
        }
        gl_FragColor = color/(kernel_sum*2.0);
}


