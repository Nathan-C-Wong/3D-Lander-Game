#pragma once

#include "ofMain.h"

class Shape {
public:
    // methods
    Shape() {}
    virtual void draw() {}
    virtual bool intersect(glm::vec3 p) { return false; }

    // data
    glm::mat4 Shape::getTransform() {
        glm::mat4 T = glm::translate(glm::mat4(1.0), position);
        glm::mat4 R = glm::rotate(glm::mat4(1.0), glm::radians(angle), glm::vec3(0, 0, 1));
        glm::mat4 S = glm::scale(glm::mat4(1.0), scale);
        return (T * R * S);

    }

    // Heading Vector that points down
    glm::vec3 heading() {
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        return glm::normalize(glm::vec3(rot * glm::vec4(0, 0, -1, 0))); 
    }

    // Heading Vector
    glm::vec3 downHeading() {
        //glm::mat4 rot = glm::rotate(glm::mat4(1.0), glm::radians(angle), glm::vec3(0, 0, 1));
        //return glm::normalize(rot * glm::vec4(glm::vec3(0, -1, 0), 1));
        //glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 0, 1));
        return glm::normalize(glm::vec3(glm::vec4(0, -1, 0, 0)));
    }

    // Draws the heading vector
    void drawLine() {
        //ofDrawLine(position, position + heading() * (888 * scale));
        glm::vec3 dir = heading();
        float length = 300.0f;  //*scale;      
        ofDrawLine(position, position + dir * length);
    }

    // Draw the down heading vector
    void drawDownLine() {
        glm::vec3 dir = downHeading();
        float length = 300.0f;  //*scale;      
        ofDrawLine(position, position + dir * length);
    }

    // No physics just forward
    //void moveForward() {
    //    position += heading() * 10;
    //}
    //// No physics just backward
    //void moveBackward() {
    //    position -= heading() * 10;
    //}

    // Data
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);
    float angle = 0;
};