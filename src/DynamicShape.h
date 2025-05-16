#pragma once

#include "ofMain.h"
#include "Shape.h"

class DynamicShape : public Shape {
public:
    DynamicShape(); // Constructor

    void integrate();   // Update physics state
    void applyForce(glm::vec3 force); //Apply force
    void applyTorque(float torqueAmount);

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