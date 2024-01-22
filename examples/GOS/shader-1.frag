const float M_Pi = 3.1415926535897932384626433832795;
uniform sampler2D MyTex;
uniform float dimen_1;
uniform float dimen_2;

uniform float sigma;
uniform float x;
uniform float size;

uniform float direc;

vec2 texSize = vec2(dimen_1, dimen_2);
//vec2 texSize = vec2(float(gl_TextureMatrix[0][1] - gl_TextureMatrix[0][0]), float(gl_TextureMatrix[0][2] - gl_TextureMatrix[0][0]));
vec2 uv = gl_FragCoord.xy / texSize;
vec4 color = vec4(0.0);
vec2 blur_dir = vec2(1.0, 0.0);


float normal_fac = 1.0/(pow(2.0 * M_Pi, 0.5) * sigma) * x;
float e_kernel = (2.0*pow(sigma, 2.0));
float kernel_sum = 0.0;
float i_max = size * sigma;

void main() {
	if (direc == 0.0) {
		blur_dir = vec2(1.0, 0.0);
	}
	if (direc == 1.0) {
		blur_dir = vec2(0.0, 1.0);
	}
    
    if ((sigma <= 0.0) || (x < 1.0) || (size <= 0.0) || (x > texSize[0]) || (x > texSize[1])) {
        gl_FragColor = texture2D(MyTex, uv);
    }
    else {
        for (float i = 0.0; i < size*sigma; i+=(1.0*x)) {
            
            float kernel = normal_fac * exp(-(pow(i, 2.0)/e_kernel));
            kernel_sum += kernel;
            
            color += (texture2D(MyTex, uv + blur_dir * (i*x)/texSize)) * kernel;
            color += (texture2D(MyTex, uv - blur_dir * (i*x)/texSize)) * kernel;
                
        }
        gl_FragColor = color/(kernel_sum*2.0);
    }
}


