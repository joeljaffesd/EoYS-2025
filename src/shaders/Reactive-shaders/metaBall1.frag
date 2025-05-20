#version 330 core

in vec3 vPos; //recieve from vert
in vec2 vUV;

out vec4 fragColor;

uniform float u_time;
uniform float onset;
uniform float cent;
uniform float flux;

// vec2 uv = vec2(vPos.xy);

//starter code
//https://www.shadertoy.com/view/wd3SzS



struct MetaBall{
	float r;
    vec2 pos;
    vec3 col;
};

vec4 BallSDF(MetaBall ball, vec2 vertex){
    float dst = (ball.r*10.0) / length(vertex - ball.pos*10.0+(onset*10.0));
    return vec4(ball.col * dst, dst);
}

vec3 renderMetaBall(vec2 vertex, float offset, float time){ //int numberBalls
	MetaBall  mbr, mbg, mbb;
   // for (int i = 0; i<numberBalls; ++i){
    // mbr.pos = 0.7 * sin(u_time*.5 + vec2(4.0, 0.5 + offset) + 6.0); mbr.r = 0.6; mbr.col = vec3(1., 0., 0.);
    // mbg.pos = 0.7 * sin(u_time*.8 + vec2(1.0, 25 + offset) + 2.0); mbg.r = 0.9; mbg.col = vec3(0., 1., 0.);
    // mbb.pos = 0.7 * sin(u_time*1.2 + vec2(3.0, 2.5 + offset) + 4.0); mbb.r = 0.75; mbb.col = vec3(0., 0., 1.);
    // mbr.pos = 0.9 * cos(time*.5 + vec2(vertex.x+offset*10000.0, vertex.y + offset) + 1000.0); mbr.r = 0.5; mbr.col = vec3(1., 0., 0.);
    // mbg.pos = 0.7 * sin(time*.8 + vec2(vertex.x+offset*2000.0, vertex.y + offset) + 200000.0); mbg.r = 0.1 ; mbg.col = vec3(0., 1., 0.);
    // mbb.pos = 0.7 * cos(time*1.2 + vec2(vertex.x+offset*3000.0, vertex.y + offset) + 4.0); mbb.r = 0.7; mbb.col = vec3(0., 0., 1.);

// Spread the balls more
    
    // ball pos
    mbr.pos = 0.5 * vec2(cos(time * 0.5 + offset * 6.0), sin(time * 0.5 + offset * 3.0));
    mbg.pos = 0.7 * vec2(sin(time * 0.8 + offset * 4.0), cos(time * 0.8 + offset * 2.0));
    mbb.pos = 0.3 * vec2(cos(time * 1.2 + offset * 5.0), sin(time * 1.2 + offset * 7.0));
    
    // radius
    mbr.r = 0.4 + 0.3 * sin(time * 0.3 + offset);
    mbg.r = 0.5 + 0.2 * cos(time * 0.4 + offset);
    mbb.r = 0.6 + 0.25 * sin(time * 0.6 + offset);
    
    // color for each ball in metaball
    mbr.col = vec3(1.0, 0.0, 0.3);
    mbg.col = vec3(1.0, 0.5, 0.2);
    mbb.col = vec3(0.0, 0.3, 1.0);
    
	
    vec4 ballr = BallSDF(mbr, vertex);
    vec4 ballg = BallSDF(mbg, vertex);
    vec4 ballb = BallSDF(mbb, vertex);
    
    float total = ballr.a + ballg.a + ballb.a;
    float threshold = total > 4.5 ? 1. : 0.;
    vec3 color = (ballr.rgb + ballg.rgb + ballb.rgb) / total;
    color *= threshold;
    return color;
}


int nMetaBalls = 10;

void main() {

     vec2 uv = vPos.xy;
     float t = (u_time)+(0.1*flux);

    float k = cos(t);
    float l = sin(t);
    float s = 0.2;//+(onset/100.0); //+ (onset / u_time);

    // for(int j=0; j<2; ++j) {
    //     uv  = abs(uv) - s;//-onset;    // Mirror
    //     uv *= mat2(k,-l,l,k); // Rotate
    //     s  *= 0.8;///(t+1);         // Scale
    // }
    // vec2 uv = vPos.xy;
    //float t = (u_time * (0.001*(flux*3 )))+onset; //+ (cent);
    //float t = (u_time * 0.01); //XXX simplify back to whats in shadertoy example
    // float t = (u_time) + onset;
    
    vec3 col = vec3(0.);

    for (int i = 0; i < nMetaBalls; ++i){

    col += renderMetaBall(uv, float(i), t);
    }

    fragColor = vec4(col, 1.0);

   
    //fragColor = .5 + .5*cos(6.28318*(40.0*length(uv))*vec4(-1,2+(u_time/500.0),3+flux,1)); //u time makes it grainy over time

}
 