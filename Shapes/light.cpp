#include "light.h"

light::light(){
    this->color = glm::vec4(0.0f);
    this->dir = glm::vec4(0.0f);
}

light::light(glm::vec4 col, glm::vec4 dir){ //Deep copy time.
    this->color = col;
    this->dir = dir;
}

void light::setColor(glm::vec4 col){
    this->color = col; 
}

pointLight::pointLight():light(){
    intensity=0.0f;
    range=0.0f;
}

pointLight::pointLight(glm::vec4 pos, glm::vec4 col, float intensity, float range):light(col,pos){
    this->intensity=intensity;
    this->range=range;
}

glm::vec4 pointLight::calculateLighting(const glm::vec4& pointPos, const glm::vec4& normal, const glm::vec4& material) const {
    // Implements pseudocode: E = max(0, n . l) * i / r^2 ; return k * E
    glm::vec4 p = this->dir; // light position
    glm::vec4 iCol = this->color; // light colour
    glm::vec4 L = p - pointPos; // p - x
    float distSq = glm::dot(L, L);

    // If outside effective range return ambient-only scaled by material
    if (range > 0.0f && distSq > (range * range)) {
        glm::vec4 ambientOnly = glm::vec4(0.0f);
        const float ambient = 0.005f; // lowered ambient
        for (int j = 0; j < 3; ++j) ambientOnly[j] = material[j] * ambient * material[3];
        ambientOnly[3] = material[3];
        return ambientOnly;
    }

    float dist = std::sqrt(distSq);
    glm::vec4 ldir = (dist > 0.0f) ? glm::vec4(glm::normalize(glm::vec3(L))) : glm::vec4(0.0f);

    float ndotl = normal[0]*ldir[0] + normal[1]*ldir[1] + normal[2]*ldir[2];
    ndotl = std::max(0.0f, ndotl);

    // Use true inverse-square attenuation (1 / r^2). Guard against zero distance.
    float atten = 0.0f;
    const float eps = 1e-6f;
    atten = intensity / (std::max(distSq, eps));

    glm::vec4 E = glm::vec4(0.0f);
    for (int j = 0; j < 3; ++j) E[j] = iCol[j] * (ndotl * atten);

    glm::vec4 result;
    // Reduce ambient so attenuation dominates at distance; incorporate material alpha
    const float ambient = 0.005f;
    for (int j = 0; j < 3; ++j) {
        float val = material[j] * E[j] + material[j] * ambient;
        // scale by material alpha so transparent materials contribute less
        val *= material[3];
        if (val > 1.0f) val = 1.0f;
        if (val < 0.0f) val = 0.0f;
        result[j] = val;
    }
    result[3] = material[3];
    return result;
}


void pointLight::setPosition(glm::vec4 pos){
    this->dir = pos;
}

void pointLight::setIntensity(float i){
    this->intensity=i;
}

