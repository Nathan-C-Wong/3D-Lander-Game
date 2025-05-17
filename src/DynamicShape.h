#pragma once

#include "ofMain.h"
#include "Shape.h"

//Danilo Makarewycz and Nathan Wong
class DynamicShape : public Shape {
public:
    DynamicShape(); // Constructor

    void integrate();   // Update physics state
    void applyForce(glm::vec3 force); //Apply force
    void applyTorque(float torqueAmount);
    void moveForward();
    void moveBackward();
    void turnRight();
    void turnLeft();
    void moveUp();
    void moveDown();
    void gravity();
    void applyLandingImpulse(float impulseStrength = 5.0f);

    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 forces;
    float mass;
    float damping;
    float lifespan;
    float birthtime;

    float forward = 20;
    float backward = -20;
    float torque;
};