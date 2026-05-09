#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aColor;

uniform sampler2D displacementMap;
uniform bool useDisplacement;
uniform float displacementScale;

out vec2 TexCoords;
out vec3 VertexColor;

void main() {
    vec3 newPos = aPos;

    if (useDisplacement) {
        float height = texture(displacementMap, aTexCoords).r;
        // Move the vertex along its normal based on the map value
        newPos += aNormal * (height * displacementScale);
    }

    TexCoords = aTexCoords;

    VertexColor = aColor;

    gl_Position = vec4(newPos, 1.0);
}