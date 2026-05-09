#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
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
};
#endif