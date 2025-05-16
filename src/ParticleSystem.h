#pragma once

#include "Particle.h"
#include <glm/gtc/random.hpp>

class ParticleSystem {
public:
    vector<Particle> particles;

    void addExplosion(glm::vec3 pos, int count = 80) {
        for (int i = 0; i < count; i++) {
            glm::vec3 vel = glm::sphericalRand(ofRandom(400, 650));   // originally 50, 150
            float lifetime = ofRandom(0.5, 2.5);            // originally 0.1, 1.5
            ofColor color = ofColor::orangeRed;

            particles.push_back(Particle(pos, vel, lifetime, color));
        }
    }

    void addSmallExplosion(glm::vec3 pos, int count = 80) {
        for (int i = 0; i < count; i++) {
            glm::vec3 vel = glm::sphericalRand(ofRandom(50, 150));   // originally 50, 150
            float lifetime = ofRandom(0.5, 1.5);            // originally 0.1, 1.5
            ofColor color = ofColor::orangeRed;
            particles.push_back(Particle(pos, vel, lifetime, color));
        }
    }

    void addPlayerExplosion(glm::vec3 pos, int count = 500) {
        for (int i = 0; i < count; i++) {
            glm::vec3 vel = glm::sphericalRand(ofRandom(50, 300));   // originally 50, 150
            float lifetime = ofRandom(1.5, 3.5);            // originally 0.1, 1.5
            ofColor color;

            // For random blue coloring (futuristic blue fuel explosion)
            ofColor color1 = ofColor::aqua;
            ofColor color2 = ofColor::aquamarine;
            ofColor color3 = ofColor::blueSteel;

            float colorChoice = ofRandom(3);
            if (colorChoice < 1) {
                color = color1;
            }
            else if (colorChoice < 2) {
                color = color2;
            }
            else if (colorChoice < 3) {
                color = color3;
            }

            particles.push_back(Particle(pos, vel, lifetime, color));
        }
    }

    void thrusterParticle(glm::vec3 pos, glm::vec3 direction, int count = 10) {
        for (int i = 0; i < count; i++) {
            // Particle velocity is backwards
            glm::vec3 vel = direction + glm::sphericalRand(ofRandom(50, 150)) * 0.5f;  // Adding a bit of randomness to the thrust direction

            // Shorter lifespan for some particles
            float lifetime = ofRandom(0.5, 1.5);

            // Color gradient of blue (futuristic fuel)
            ofColor color = ofColor::aqua;

            color.a = ofRandom(180, 255);

            // Push new particle with updated direction and color
            particles.push_back(Particle(pos, vel, lifetime, color));
        }
    }

    void backwardsThrusterParticle(glm::vec3 pos, glm::vec3 direction, int count = 10) {
        for (int i = 0; i < count; i++) {
            // Particle velocity is backwards
            glm::vec3 vel = direction + glm::sphericalRand(ofRandom(50, 150)) * 0.5f;  // Adding a bit of randomness to the thrust direction

            // Shorter lifespan for some particles
            float lifetime = ofRandom(0.1, 0.2);

            // Color gradient of aqua blue (futuristic fuel)
            ofColor color = ofColor::aqua;

            color.a = ofRandom(180, 255);

            // Push new particle with updated direction and color
            particles.push_back(Particle(pos, vel, lifetime, color));
        }
    }


    void update(float dt) {
        for (int i = particles.size() - 1; i >= 0; i--) {
            particles[i].update(dt);
            if (particles[i].isDead()) {
                particles.erase(particles.begin() + i);
            }
        }
    }

    void draw() {
        for (auto& p : particles) {
            p.draw();
        }
    }
};
