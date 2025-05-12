
// basic shader toy example //removed use of iResolution because we handle that in vUV
#version 330 core

uniform vec3 iResolution; // faking this data
uniform float iTime;

// Pass from vertex shader -- input provided by alloli 
in vec2 vUv;

out vec4 fragColor;



void main()
{
    // Reconstruct UV
    vec2 uv = vUv;

    vec3 col = 0.5 + 0.5 * cos(10000.0 * iTime + uv.xyx * 20.0 + vec3(0.0, 2.0, 4.0));


     // ADD A SMALL DEPENDENCY ON FRAGMENT POSITION
    col += vec3(vUv.x * 0.01, vUv.y * 0.01, 0.0);

    fragColor = vec4(col, 1.0);
}