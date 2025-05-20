#version 330 core

in vec3 vPos; //recieve from vert
in vec2 vUV;

out vec4 fragColor;

uniform float u_time;
uniform float onset;
uniform float cent;
uniform float flux;

/* looking at simple math i can integrate:
- https://www.reddit.com/r/math/comments/15stx6b/what_symmetries_if_any_underlie_basic_calculus/
*/

void main() {
    vec2 uv = 0.03 * vPos.xy;
    float t = (u_time * 0.01) + onset;
    
    float k = cos(t) - (length(uv.x/4));
    float l = sin(t) + (length(uv.y/2));
    float s = 0.2+(onset/30.0) / mod(t,20.0); // You need to define s before using it
    
    // 
    // uv = vec2(
    //     abs(uv.x) - s,
    //     (uv.y > 0.0) ? uv.y - s : uv.y + s
    // );

    for(int i=0; i<80; ++i) {
        uv = abs(uv) - s; // Mirror
        uv *= mat2(k+s,-l,l,k); // Rotate
        s *= .95156; // Scale
    }

    uv = vec2(
        abs(uv.x) - s,
        (uv.y > 0.0) ? uv.y - s : uv.y + s //trying assumetrical logic 
    );

    float x = .5 + .5*cos(t*(40.0*length(uv)));

    fragColor = .5 + .5*cos(6.28318*(300.0*length(uv.x))*vec4(3,3,30+flux,1)); // u time makes it grainy over time
}