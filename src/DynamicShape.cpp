#include "DynamicShape.h"

//Physics 
DynamicShape::DynamicShape() :
    velocity(0, 0, 0),
    acceleration(0, 0, 0),
    forces(0, 0, 0),
    lifespan(0),
    birthtime(0),
    damping(0.99f),
    mass(1.0f),
    torque(0) {
}

void DynamicShape::applyForce(glm::vec3 force) {
    forces += force;
}

void DynamicShape::applyTorque(float torqueAmount) {
    torque = torqueAmount;
}

void DynamicShape::integrate() {

    // (1) update position from velocity and time interval
    // (2) update velocity (based on acceleration
    // (3) multiply final result by the damping factor to sim drag
    //

    if (ofGetFrameRate() > 0) {
        float dt = 1.0 / ofGetFrameRate();

        position += velocity * dt;

        glm::vec3 accel = acceleration;
        accel += (forces * (1.0 / mass));
        velocity += accel * dt;

        velocity *= damping;

        forces = glm::vec3(0, 0, 0);
    }
    // Apply torque to change the angle (for turning)
    angle += torque;
    torque = torque * 0.975;   // 0.975 mimics a good drift

}

void DynamicShape::moveForward() {
    applyForce(heading() * 1.5 * 20);
}

void DynamicShape::moveBackward() {
    applyForce(-heading() * 1.5 * 20);
}

void DynamicShape::turnRight() {
    //angle += 5;
    applyTorque(-1.7f);
}

void DynamicShape::turnLeft() {
    //angle -= 5;
    applyTorque(1.7f);
}

void DynamicShape::moveUp() {
    applyForce(-downHeading() * 20);
}

void DynamicShape::moveDown() {
    applyForce(downHeading() * 50);
}