#include "Camera.h"

Camera::Camera()
    : position(0.0f, 5.0f, 10.0f),
      target(0.0f, 0.0f, 0.0f),
      up(0.0f, 1.0f, 0.0f),
      fov(60.0f),
      aspect(16.0f / 9.0f),
      nearPlane(0.05f),
      farPlane(1000.0f)
{}

Camera::Camera(float fovDegrees, float aspect, float nearPlane, float farPlane)
    : position(0.0f, 5.0f, 10.0f),
      target(0.0f, 0.0f, 0.0f),
      up(0.0f, 1.0f, 0.0f),
      fov(fovDegrees),
      aspect(aspect),
      nearPlane(nearPlane),
      farPlane(farPlane)
{}

void Camera::setPosition(const glm::vec3& pos) { position = pos; }
void Camera::setTarget(const glm::vec3& t)     { target = t; }
void Camera::setUp(const glm::vec3& u)          { up = u; }
void Camera::setAspect(float a)                 { aspect = a; }

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, target, up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}

glm::mat4 Camera::getViewProjection() const {
    return getProjectionMatrix() * getViewMatrix();
}

glm::vec3 Camera::getPosition() const {
    return position;
}

glm::vec3 Camera::getForward() const {
    return glm::normalize(target - position);
}
