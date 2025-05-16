#pragma once

#include "ofMain.h"

class Particle {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
    float age;
    ofColor color;

    Particle(glm::vec3 pos, glm::vec3 vel, float life, ofColor c) {
        position = pos;
        velocity = vel;
        lifetime = life;
        age = 0;
        color = c;
    }

    void update(float dt) {
        position += velocity * dt;
        age += dt;
        velocity *= 0.95f;
    }

    void draw() {
        float alpha = ofMap(age, 0, lifetime, 255, 0, true);
        ofSetColor(color.r, color.g, color.b, alpha);
        ofDrawCircle(position, 2);
    }

    bool isDead() {
        return age >= lifetime;
    }
};
