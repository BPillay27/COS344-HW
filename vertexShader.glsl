#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aColor;

uniform sampler2D displacementMap;
uniform bool useDisplacement;
uniform float displacementScale;

// Directional Light Uniforms
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform float uLightIntensity;

// Point Light Uniforms
uniform vec3 uPointLightPos;
uniform vec3 uPointLightColor;
uniform float uPointLightIntensity;
uniform float uPointLightRange;

uniform mat3 uNormalMatrix;
uniform mat4 uMVP;

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

    // Calculate directional lighting
    vec3 N = normalize(uNormalMatrix * aNormal);
    vec3 L = normalize(uLightDir);
    float ndotl = max(0.0, dot(N, L));
    
    // Ambient lighting (visible even in shadows)
    vec3 ambientLight = vec3(0.7);
    
    // Directional light contribution
    vec3 directionalLight = uLightColor * (ndotl * uLightIntensity);
    
    // Point light calculations
    vec3 pointLightVec = uPointLightPos - newPos;
    float distToPointLight = length(pointLightVec);
    
    vec3 pointLightDir = normalize(pointLightVec);
    float pointNdotl = max(0.0, dot(N, pointLightDir));
    
    // Point light with attenuation (inverse square law with smoothing)
    float attenuation = 1.0 / (1.0 + 0.1 * (distToPointLight * distToPointLight));
    if (distToPointLight > uPointLightRange) {
        attenuation = 0.0;
    }
    
    vec3 pointLight = uPointLightColor * (pointNdotl * uPointLightIntensity * attenuation);
    
    // Combine all lighting
    vec3 totalLight = ambientLight + directionalLight + pointLight;
    
    // Apply combined lighting to material color
    vec3 finalColor = aColor * totalLight;
    
    // Clamp color values to [0, 1]
    finalColor = clamp(finalColor, vec3(0.0), vec3(1.0));
    
    VertexColor = finalColor;

    gl_Position = uMVP * vec4(newPos, 1.0);
}