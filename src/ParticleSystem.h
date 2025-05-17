#pragma once
#include "ofMain.h"
#include "Particle.h"
/*
*  Danilo Makarewycz and Nathan Wong
*/

class ParticleSystem {
public:
    std::vector<Particle> particles;
    ofVec3f emitterPos;
    bool active = false;
    bool continuous = false;

    void setPosition(ofVec3f pos) {
        emitterPos = pos;
    }

    void start(int numParticles = 100) {
        active = true;
        particles.clear();
        if (continuous) {
            for (int i = 0; i < numParticles; ++i) {
                ofVec3f vel = ofVec3f(ofRandom(-0.5, 0.5), ofRandom(-10, -20), ofRandom(-0.5, 0.5));
                ofColor color = ofColor::orangeRed;
                float lifespan = ofRandom(1.5, 2.5);
                particles.emplace_back(emitterPos, vel, color, lifespan);
            }
            return;
        }
        continuous = false;
        particles.clear();

        for (int i = 0; i < numParticles; ++i) {
            addParticle();
        }
    }

    void startContinuous() {
        active = true;
        continuous = true;
    }

    void stop() {
        continuous = false;
    }

    void addParticle() {
        ofVec3f vel = ofVec3f(ofRandom(-5, 5), ofRandom(0, 10), ofRandom(-5, 5));
        ofColor color = ofColor::orangeRed;
        float lifespan = ofRandom(1.0, 2.0);
        particles.push_back(Particle(emitterPos, vel, color, lifespan));
    }

    void update(float dt) {
        if (!active) return;

        if (continuous) {
            for (int i = 0; i < 5; i++) { // 5 particles per frame
                addParticle();
            }
        }

        for (auto& p : particles) {
            p.update(dt);
        }

        particles.erase(std::remove_if(particles.begin(), particles.end(),
            [](Particle& p) { return !p.alive; }), particles.end());

        if (!continuous && particles.empty()) {
            active = false;
        }
    }

    void draw() {
        if (!active) return;
        for (auto& p : particles) {
            p.draw();
        }
    }
};

