#pragma once

#include "Shape.h"
#include "ofMain.h"
/*
* Danilo Makarewycz and Nathan Wong
*/
class BoxShape : public Shape {
public:
    float width;
    float height;
    float depth;
    BoxShape() {
        width = 10;
        height = 10;
        depth = 10;
    }
    BoxShape(float w, float h, float d) {
        width = w;
        height = h;
        depth = d;
    }
    void draw() {
        ofPushMatrix();
        ofMultMatrix(getTransform());
        ofSetColor(ofColor::white);
        ofDrawBox(-width / 2, -height / 2, -depth / 2, width, height, depth);
        ofPopMatrix();
    }
    bool intersect(glm::vec3 p) {
        return true;
    }

};