#version 330 core

in vec3 vPos;
out vec4 fragColor;

uniform float u_time;

void main() {
    // attempting to map spherical coords to fractal coords
    vec2 c = vPos.xy * 0.25 + vec2(0.2 * sin(u_time * 0.1), 0.2 * cos(u_time * 0.1));


    vec2 z = c;
    int maxIterations = 50;
    int i;
    for (i = 0; i < maxIterations; ++i) {
        float x = (z.x * z.x - z.y * z.y) + c.x;
        float y = (2.0 * z.x * z.y) + c.y;
        if ((x * x + y * y) > 4.0) break;
        z = vec2(sin(x*u_time), y);
    }

    float color = float(i) / float(maxIterations);

    fragColor = vec4(vec3(color), 1.0); //
}
