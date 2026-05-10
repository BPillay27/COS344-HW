#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

class light{
    protected:
        glm::vec4 color;
        glm::vec4 dir;
    public:
        light();
        light(glm::vec4 col, glm::vec4 dir);
        virtual glm::vec4 calculateLighting(const glm::vec4& pointPos, const glm::vec4& normal, const glm::vec4& material) const =0;
        void setColor(glm::vec4 col);

};

class pointLight:public light{
    protected:
        float intensity;
        float range;
    public:
        pointLight();
        pointLight(glm::vec4 pos, glm::vec4 col, float intensity, float range);
        glm::vec4 calculateLighting(const glm::vec4& pointPos, const glm::vec4& normal, const glm::vec4& material) const;
        void setPosition(glm::vec4 pos);
        void setIntensity(float i);
        
        // Getters and setters for shader uniforms
        glm::vec4 getPosition() const { return dir; }
        glm::vec4 getColor() const { return color; }
        float getIntensity() const { return intensity; }
        float getRange() const { return range; }
        void setColor(glm::vec4 col) { color = col; }
};

class directionalLight : public light {
    protected:
        float intensity;
    public:
        directionalLight();
        directionalLight(glm::vec4 dir, glm::vec4 col, float intensity);
        
        // Lighting calculation (no attenuation)
        glm::vec4 calculateLighting(const glm::vec4& pointPos, const glm::vec4& normal, const glm::vec4& material) const override;
        
        void setDirection(glm::vec4 dir);
        
        // Rotates the light direction around the X axis using radians
        void rotateX(float radians);
        
        // Getters for shader uniforms
        glm::vec4 getDirection() const { return dir; }
        glm::vec4 getColor() const { return color; }
        float getIntensity() const { return intensity; }
};
#endif