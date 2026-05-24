#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 VertexColor;

uniform sampler2D colorMap;     
uniform sampler2D alphaMap;    

uniform vec4 uBaseColor;       
uniform bool useColor;          // Toggle for colorMap
uniform bool useAlphaMap;       // Toggle for alphaMap
uniform bool useGrayscale;      // Toggle for grayscale filter

void main() {
    // Default to solid white so vertex colors multiply correctly
    vec4 result = vec4(1.0, 1.0, 1.0, 1.0);

    if (useColor) {
        vec4 sampledTex = texture(colorMap, TexCoords);
        result.rgb *= sampledTex.rgb;
    }

    if (useAlphaMap) {
        float maskValue = texture(alphaMap, TexCoords).r;
    }

    // Apply CPU per-vertex color
    result.rgb *= VertexColor;

    // Apply grayscale filter if enabled
    if (useGrayscale) {
        float gray = dot(result.rgb, vec3(0.2126, 0.7152, 0.0722));
        result.rgb = vec3(gray);
    }

    FragColor = result;
}