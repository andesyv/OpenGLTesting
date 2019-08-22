#ifndef CAMERA_H
#define CAMERA_H

#include "matrix4x4.h"
#include "vector3d.h"

class Camera
{
public:
    Camera();

    void pitch(float degrees);
    void yaw(float degrees);
    void updateRightVector();
    void updateForwardVector();
    void update();

    glm::Matrix4x4 mViewMatrix;
    glm::Matrix4x4 mProjectionMatrix;

    void setPosition(const glm::Vector3D &position);

    void setSpeed(float speed);
    void updateHeigth(float deltaHeigth);
    void moveRight(float delta);

    glm::Vector3D position() const;
    glm::Vector3D up() const;

private:
    glm::Vector3D mForward{0.f, 0.f, 1.f};
    glm::Vector3D mRight{-1.f, 0.f, 0.f};
    glm::Vector3D mUp{0.f, 1.f, 0.f};

    glm::Vector3D mPosition{0.f, 0.f, 0.f};
    float mPitch{0.f};
    float mYaw{0.f};

    glm::Matrix4x4 mYawMatrix;
    glm::Matrix4x4 mPitchMatrix;

    float mSpeed{0.f}; //camera will move by this speed along the mForward vector
};

#endif // CAMERA_H
