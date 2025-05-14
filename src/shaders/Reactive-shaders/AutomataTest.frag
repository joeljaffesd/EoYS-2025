#version 330 core

in vec3 vPos; //recieve from vert
out vec4 fragColor;

uniform float u_time;
uniform float onset;
uniform float cent;
uniform float flux;


// *** STARTER CODE INSPIRED BY : https://www.shadertoy.com/view/4lSSRy *** //

void main() {
    vec2 uv = 0.275 * vPos.xy;
    float t = (u_time * (0.001*(flux*2)))+onset; //+ (cent);
    //t += flux;
    //t +=cent;
    float k = cos(t);
    float l = sin(t);
    float s = 0.2 + (onset / u_time);

    for(int i=0; i<64; ++i) {
        uv  = abs(uv) - s*flux;//-onset;    // Mirror
        uv *= mat2(k,-l,l,k); // Rotate
        s  *= .95156;///(t+1);         // Scale
    }

    float x = .5 + .5*cos(6.28318*(40.0*length(uv)));

    

    //fragColor =  vec4(vec3(x),1);
     //fragColor =  x* vec4(1,2*flux,3,1);
   
    fragColor = .5 + .5*cos(6.28318*(40.0*length(uv))*vec4(-1,2,3+flux,1));

}

