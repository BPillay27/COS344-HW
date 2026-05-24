#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aColor;

uniform sampler2D displacementMap;
uniform bool useDisplacement;
uniform float displacementScale;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

// Camera uniform for view direction
uniform vec3 uCameraPos;

// Directional Light Uniforms
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform float uLightIntensity;

// Point Light Uniforms
uniform vec3 uPointLightPos;
uniform vec3 uPointLightColor;
uniform float uPointLightIntensity;
uniform float uPointLightRange;

// Blinn-Phong material properties
uniform float uShininess;

uniform mat3 uNormalMatrix;

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

    // Blinn-Phong Lighting Model
    vec3 N = normalize(uNormalMatrix * aNormal);
    vec3 V = normalize(uCameraPos - newPos);
    
    // Ambient lighting (visible even in shadows)
    vec3 ambientLight = vec3(0.45);
    
    // ===== DIRECTIONAL LIGHT =====
    vec3 L_dir = normalize(uLightDir);
    float ndotl_dir = max(0.0, dot(N, L_dir));
    
    // Diffuse component
    vec3 directionalDiffuse = uLightColor * (ndotl_dir * uLightIntensity);
    
    // Specular component (Blinn-Phong)
    vec3 H_dir = normalize(L_dir + V);
    float ndoth_dir = max(0.0, dot(N, H_dir));
    vec3 directionalSpecular = uLightColor * (pow(ndoth_dir, uShininess) * uLightIntensity * 0.5);
    
    vec3 directionalLight = directionalDiffuse + directionalSpecular;
    
    // ===== POINT LIGHT =====
    vec3 pointLightVec = uPointLightPos - newPos;
    float distToPointLight = length(pointLightVec);
    
    vec3 L_point = normalize(pointLightVec);
    float ndotl_point = max(0.0, dot(N, L_point));
    
    // Point light with attenuation (inverse square law with smoothing)
    float attenuation = 1.0 / (1.0 + 0.1 * (distToPointLight * distToPointLight));
    if (distToPointLight > uPointLightRange) {
        attenuation = 0.0;
    }
    
    // Diffuse component
    vec3 pointDiffuse = uPointLightColor * (ndotl_point * uPointLightIntensity * attenuation);
    
    // Specular component (Blinn-Phong)
    vec3 H_point = normalize(L_point + V);
    float ndoth_point = max(0.0, dot(N, H_point));
    vec3 pointSpecular = uPointLightColor * (pow(ndoth_point, uShininess) * uPointLightIntensity * attenuation * 0.5);
    
    vec3 pointLight = pointDiffuse + pointSpecular;
    
    // Combine all lighting
    vec3 totalLight = ambientLight + directionalLight + pointLight;
    
    // Apply combined lighting to material color
    vec3 finalColor = aColor * totalLight;
    
    // Clamp color values to [0, 1]
    finalColor = clamp(finalColor, vec3(0.0), vec3(1.0));
    
    VertexColor = finalColor;

    gl_Position = uProjection * uView * uModel * vec4(newPos, 1.0);
}