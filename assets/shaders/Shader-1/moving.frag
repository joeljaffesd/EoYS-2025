#version 330 core

in vec3 vPos; //recieve from vert
out vec4 fragColor;

uniform float u_time;

void main() {
    float brightness = 0.5 + 0.5 * sin(u_time + vPos.x * 2.0 + vPos.y * 2.0);

    vec3 color = vec3(
        0.7 + 0.5 * sin(u_time + tan(vPos.x)),
        0.5 + 0.7 * cos(u_time + vPos.y),
        0.3 + 0.5 * sin(u_time + vPos.z)
    );

    fragColor = vec4(color * brightness, 1.0);
}
