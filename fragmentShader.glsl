#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 VertexColor;

uniform sampler2D colorMap;     
uniform sampler2D alphaMap;    

uniform vec4 uBaseColor;       
uniform bool useColor;          // Toggle for colorMap
uniform bool useAlphaMap;       // Toggle for alphaMap

void main() {

    vec4 result = uBaseColor;

    if (useColor) {
        vec4 sampledTex = texture(colorMap, TexCoords);
        // Multiply RGB by texture RGB. Do NOT use the color texture alpha channel
        // to avoid unintentionally dimming or making parts translucent.
        result.rgb *= sampledTex.rgb;
    }


    if (useAlphaMap) {
        float maskValue = texture(alphaMap, TexCoords).r;
        // Binary cutoff: discard below threshold and force opaque otherwise
        if (maskValue < 0.5) discard;
        result.a = 1.0;
    }

    // Apply CPU per-vertex color (lighting) only — no GPU lighting calculations.
    result.rgb *= VertexColor;

    FragColor = result;
}