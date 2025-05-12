#version 330 core

in vec3 vPos; //recieve from vert
out vec4 fragColor;

uniform float u_time;
uniform float onset;
uniform float cent;

void main() {
    float t = u_time+(u_time * cent);
    float brightness = tan(t + vPos.x * 2.0 + vPos.y * 2.0);

    vec3 color = vec3(
        0.7 + 0.5 * pow(t, tan(vPos.x)) * onset,
        0.5 + 0.7 * cos(t + vPos.y) * onset,
        0.3 + 0.5 * sin(t*t*vPos.y + vPos.z) * onset
    );

    fragColor = vec4(color * brightness, 1.0);
}
