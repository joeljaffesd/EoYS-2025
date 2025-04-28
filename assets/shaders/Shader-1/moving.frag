#version 330 core
out vec4 fragColor;

uniform float u_time;

void main() {
    float brightness = 0.5 + 0.5 * sin(u_time);
    fragColor = vec4(brightness/u_time, tan(brightness), cos(brightness*u_time), 1.0);
}
