#version 330 core

in vec2 coords;

uniform sampler2D atlas;

uniform bool Edge;
uniform bool Blur;
uniform bool Posterize;
uniform bool MegaWarp;
uniform bool Warp;

uniform bool Invert;

uniform float time;

in vec4 Tint;

out vec4 FragColor;

const float offsetX = 1.0 / 800.0;
const float offsetY = 1.0 / 600.0;  

void main() {

    vec2 texCoords = coords;

    if (MegaWarp) {
        texCoords.x += sin(542353 * texCoords.y + texCoords.x) * 0.2;
        texCoords.y += sin(time + texCoords.y + texCoords.x * 23457) * 0.1;
    }

    if (Warp) {
        texCoords.x += sin(13 * texCoords.y + texCoords.x) * 0.2;
        texCoords.y += sin(time + texCoords.y + texCoords.x * 7) * 0.1 - 0.1;
    }

    // adapted from https://learnopengl.com/Advanced-OpenGL/Framebuffers
	vec2 offsets[9] = vec2[](
        vec2(-offsetX,  offsetY), // top-left
        vec2(0.0f,    offsetY), // top-center
        vec2(offsetX,  offsetY), // top-right
        vec2(-offsetX,  0.0f),   // center-left
        vec2(0.0f,    0.0f),   // center-center
        vec2(offsetX,  0.0f),   // center-right
        vec2(-offsetX, -offsetY), // bottom-left
        vec2(0.0f,   -offsetY), // bottom-center
        vec2(offsetX, -offsetY)  // bottom-right    
    );

    vec4 sampleTex[9];
    for(int i = 0; i < 9; i++) {
        sampleTex[i] = texture(atlas, texCoords.st + offsets[i]);
    }

    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

    float effects = 0.0;

    if (Edge) {
        float edgeKernel[9] = float[](
            1, 1, 1,
            1,  -8, 1,
            1, 1, 1
        );

        effects += 1.0;

        for (int i = 0; i < 9; i++) {
            color += sampleTex[i] * edgeKernel[i];
        }
    }

    if (Blur) {
        float blurKernel[9] = float[](
            1.0 / 16, 2.0 / 16, 1.0 / 16,
            2.0 / 16, 4.0 / 16, 2.0 / 16,
            1.0 / 16, 2.0 / 16, 1.0 / 16  
        );

        effects += 1.0;

        for (int i = 0; i < 9; i++) {
            color += sampleTex[i] * blurKernel[i];
        }
    }


    if (effects != 0.0) {
        color /= effects;
    }

    if (Posterize) {
        if (effects == 0.0) {
            color = texture(atlas, texCoords);
            effects = 1.0;
        }    
    
        const float gamma = 0.7;
        const float colors = 8.0;

        vec3 posterized = pow(color.rgb, vec3(gamma));
        posterized = floor(posterized * colors) / colors;
        posterized = pow(posterized, vec3(1.0 / gamma));
        
        color.rgb = posterized;
    }

    if (Invert) {
        if (effects == 0.0) {
            color = texture(atlas, texCoords);
            effects = 1.0;
        }

        color = vec4(1.0 - color.rgb, 1.0);
    }

    if (effects == 0.0) {
	    FragColor = texture(atlas, texCoords) * Tint;
    }
    else {
        FragColor = color * Tint;
    }
}