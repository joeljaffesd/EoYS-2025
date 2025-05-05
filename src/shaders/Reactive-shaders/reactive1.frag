#version 330 core

in vec3 vPos; //recieve from vert
out vec4 fragColor;

uniform float u_time;
uniform int onset;
//uniform float cent;

void main() {
    float brightness = tan(u_time + vPos.x * 2.0 + vPos.y * 2.0);

    vec3 color = vec3(
        (0.3+(0.4*onset)+ 0.5 * pow(u_time, tan(vPos.x))),
        (0.3+(0.4*onset) + 0.7 * cos(u_time + vPos.y*onset)),
       (0.3+(0.4*onset) + 0.5 * sin(u_time*u_time*vPos.y + vPos.z))
    );

    fragColor = vec4(color * brightness, (1.0*onset));
}
