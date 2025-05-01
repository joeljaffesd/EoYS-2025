#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 al_ModelViewMatrix;
uniform mat4 al_ProjectionMatrix;

out vec2 vUv;

void main() {
    // Calculate spherical coordinates for UV mapping: -- otherwise values are unusable --
    float longitude = atan(position.z, position.x);
    float latitude = asin(position.y / length(position));

    // Normalize to [0,1]
    vUv = vec2(longitude / 3.14159265359, latitude / (3.14159265359 * 0.5));
    vUv = vUv * 0.5 + 0.5;

    gl_Position = al_ProjectionMatrix * al_ModelViewMatrix * vec4(position, 1.0);
}
