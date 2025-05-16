#pragma once
#include "ofMain.h"

class Particle {
public:
    ofVec3f position;
    ofVec3f velocity;
    ofColor color;
    float lifespan;  
    float age = 0;
    bool alive = true;

    Particle(ofVec3f pos, ofVec3f vel, ofColor c, float life)
        : position(pos), velocity(vel), color(c), lifespan(life) {
    }

    void update(float dt) {
        if (!alive) return;

        velocity += ofVec3f(0, -9.8 * dt, 0); // gravity
        position += velocity * dt;
        age += dt;

        if (age >= lifespan) alive = false;
    }

    void draw() {
        if (!alive) return;
        float alpha = ofMap(age, 0, lifespan, 255, 0);
        ofSetColor(color.r, color.g, color.b, alpha);
        ofDrawSphere(position, 0.5);
    }
};
