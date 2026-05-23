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

    vec4 result = uBaseColor;

    if (useColor) {
        vec4 sampledTex = texture(colorMap, TexCoords);
        result.rgb *= sampledTex.rgb;
    }


    if (useAlphaMap) {
        float maskValue = texture(alphaMap, TexCoords).r;

    }

    // Apply CPU per-vertex color (lighting) only — no GPU lighting calculations.
    result.rgb *= VertexColor;
    
    // Apply grayscale filter if enabled
    if (useGrayscale) {
        // Standard NTSC/REC.709 weights for human perception
        float gray = dot(result.rgb, vec3(0.2126, 0.7152, 0.0722));
        result.rgb = vec3(gray);
    }

    FragColor = result;
}