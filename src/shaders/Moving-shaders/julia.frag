#version 330 core

uniform float u_time;
in vec3 vPos;
out vec4 fragColor;

vec2 rotate(vec2 p, float a) {
    float c = cos(a);
    float s = sin(a);
    return vec2(c * p.x - s * p.y, s * p.x + c * p.y);
}

void main() {
    // Map from sphere's 3D position to 2D fractal space
    vec2 uv = vec2(
    atan(vPos.z, vPos.x) / 3.1415926,          // longitude
    acos(clamp(vPos.y / length(vPos), -1.0, 1.0)) / 3.1415926  // latitude
);
uv = uv * 2.0 - 1.0; // map from [0,1] to [-1,1]

    // Animate zoom and offset (scroll effect)
    float zoom = 0.75 + 0.25 * sin(u_time * 0.1);
    uv *= zoom;

    // Scroll slowly
    uv.x += 0.3 * sin(u_time * 0.07);
    uv.y += 0.3 * cos(u_time * 0.11);

    // Morphing Julia constant
    vec2 c = vec2(
        0.5 * sin(u_time * 0.3),
        0.5 * cos(u_time * 0.2)
    );

    vec2 z = uv;
    float m = 0.0;

    // Standard Julia iteration
    for (int i = 0; i < 100; ++i) {
        z = vec2(
            z.x * z.x - z.y * z.y,
            2.0 * z.x * z.y
        ) + c;

        if (length(z) > 4.0) break;
        m += 1.0;
    }

    // Smooth shading
    float color = sqrt(m / 100.0);
    vec3 finalColor = vec3(color * 0.9, color * 0.8, color);

    fragColor = vec4(finalColor, 1.0);
}
