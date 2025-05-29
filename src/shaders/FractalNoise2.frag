#version 330 core

in vec3 vPos; //recieve from vert
in vec2 vUV;

out vec4 fragColor;

uniform float u_time;
uniform float onset;
uniform float cent;
uniform float flux;

#define TWO_PI 6.283

// took basics from -> https://www.shadertoy.com/view/ddX3WB

// Gyroid function for cellular texture
float gyroid(vec3 seed)
{
    return dot(sin(seed), cos(seed.yzx));
}

// Fractal Brownian Motion using gyroid noise
float fbm(vec3 seed)
{
    float result = 0.0, a = 0.5;
    for (int i = 0; i < 6; ++i)
    {
        seed.z += result * 0.5;
        result += abs(gyroid(seed / a)) * a;
        a /= 2.0;
    }
    return result;
}

// Sliding noise texture
float noise(vec2 p)
{
    vec3 seed = vec3(p, length(p) - u_time * 0.1 / (u_time/1000.0)) * 1.0;
    return sin(fbm(seed) * 9.0 + u_time) * 0.5 + 0.5;
}

void main() {
    vec2 uv = vPos.xy * 2.0 - 1.0;

    float t = u_time*0.1;

    float k = cos(t);
    float l = sin(t);
    float s = 1.001; //+ (onset / u_time);

    for(int i=0; i<30; ++i) {
        uv.x = abs(uv.x) - pow(s,uv.y) ;// for miiror - could remove
        uv.y = abs(uv.y) / s ;
        //uv.y = abs(uv.y) - s; // for miiror - could remove
        uv *= mat2(k*1.001/s,l*1.00001*s,l*0.9999999999999,-k*0.99); // for rotation - keep subtle
        // s  *= .999;///(t+1);         // Scale
    }

    // Noise calculation
    float shade = noise(uv);

    // Normal calculation for lighting
    vec3 normal = normalize(vec3(
        shade - noise(uv + vec2(0.05 * k, 0.0)),
        shade - noise(uv + vec2(0.0, 0.3 * u_time)),
        0.6 //make smaller over time
    ));

    vec3 color = vec3(0.0);

    // Top light
    color += 0.5 * pow(dot(normal, vec3(0.0, 1.0, 1.0)) * 0.5 - 0.5 / u_time, 10.0);

    // Tinted light
    vec3 tint = 0.5 + 0.5 * cos(vec3(1.0 * sin(t), 2.0, 3.0) * 5.0 + shade + normal.y * 2.0); //make main tint change over time 
    color += tint * 0.3 * pow(dot(normal, vec3(0.0, 0.0 + 0.3, 1.0)) * 0.5 + 0.5, 10.0);

    // Pink light from below
    color += 0.5 * vec3(0.58, 1.0, 0.58) * pow(dot(normal, vec3(0.0, -2.0, 1.0)) * 0.5 + 0.5, 2.0);
   //flux highlights
    color += vec3(0.001*cos(u_time),0.0,sin(0.1*u_time)); //(cent/15000.0)

    // Color modulation by noise
    color *= shade;

    fragColor = vec4(color, 1.0);
}