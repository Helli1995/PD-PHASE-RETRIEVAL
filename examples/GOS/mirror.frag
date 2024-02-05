uniform sampler2D MyTex;
uniform float X;
uniform float num;
uniform float feedback;
uniform float dimen_1;
uniform float dimen_2;

vec2 texSize = vec2(dimen_1, dimen_2);
vec2 coord = gl_FragCoord.xy / texSize;
void main ()
{
	
    float X = X/(dimen_1);
    //float Y = Y/(1000.0);
    
    vec4 color = vec4(0.0);
    if ((X <= 0.0) || (num <= 0.0) || (feedback < 0.0)) {
        gl_FragColor = texture2D(MyTex, coord);
    }
    else {
        
        for (float i = 0.0; i <= num*X; i+= X) {
            vec2 reflector = vec2(0.0, i);
            vec2 mirror = reflect(coord, reflector);
            //mirror = vec2(mirror[0], mirror[1]);
            color += texture2D(MyTex, mirror) * feedback;
        }
        color += texture2D(MyTex, coord);
        if (num<=2.0) {
            gl_FragColor = color/(num+1.0);
        }
        else {
            gl_FragColor = color/num;
        }
    }
}
