#include "Shape.h"
#include "BoxShape.h"
#include "ofMain.h"
#include  "ofxAssimpModelLoader.h"

/*
* Danilo Makarewycz and Nathan Wong
*/
class Player : public Shape {
public:

    BoxShape playerBox;
    //Box playerBox2;
    //ParticleEmitter playerEmitter;
    ofxAssimpModelLoader playerModel;
    float speed = 10;
    bool toggleHeading = false;

    Player() {
        //scale = glm::vec3(0.05);
        playerModel.loadModel("geo/shipTest5.obj");
        position = glm::vec3(0, 300, 0);
        playerModel.setScale(0.05, 0.05, 0.05);

        ofMesh tempMesh = playerModel.getMesh(0);
        glm::vec3 min = tempMesh.getVertex(0);
        glm::vec3 max = tempMesh.getVertex(0);
        for (int i = 0; i < tempMesh.getNumVertices(); i++) {
            glm::vec3 currVertex = tempMesh.getVertex(i);
            min.x = glm::min(min.x, currVertex.x);
            min.y = glm::min(min.y, currVertex.y);
            min.z = glm::min(min.z, currVertex.z);
            max.x = glm::max(max.x, currVertex.x);
            max.y = glm::max(max.y, currVertex.y);
            max.z = glm::max(max.z, currVertex.z);
        }
        glm::vec3 dimensions = (max - min) * 0.01;
        //min /= 4;
        //max /= 4;

        playerBox = BoxShape(dimensions.x, dimensions.y, dimensions.z);
        //playerBox2 = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
        playerBox.position = position;
    }
    void draw() {
        playerModel.drawFaces();
        ofNoFill();
        ofDrawBox(position.x, position.y, position.z, playerBox.width, playerBox.height, playerBox.depth);
        //Octree::drawBox(playerBox2);
        ofFill();
    }
    bool inside(glm::vec3 p) {
        return true;
    }
    void update() {
        playerBox.position = position;
        playerBox.angle = angle;
        playerBox.scale = scale;

        playerModel.setPosition(position.x, position.y, position.z);
        playerModel.setRotation(0, angle, 0, 1, 0);

        //integrate();
    }
};