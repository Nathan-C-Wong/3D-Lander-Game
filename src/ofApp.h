#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxAssimpModelLoader.h"
#include "Octree.h"
#include <glm/gtx/intersect.hpp>
#include "DynamicShape.h"
#include "ParticleSystem.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		 //Restart game written by Danilo Makarewycz 
		void ofApp::restartGame() {
			theCam = &cam;
			collided = false;
			resolvingCollision = false;
			onGround = false;
			wasOnGround = false;
			crashDetected = false;
			currentFuel = 120.0f;
			isThrusting = false;
			explosionTriggered = false;
			lander.setPosition(0, 500, -250);
			spaceShip.position = glm::vec3(0, 500, -250);
			colBoxList.clear();
			bGameOver = false;
			lander.setScale(0.01, 0.01, 0.01);
			spaceShip.velocity = glm::vec3(0, 0, 0);
			score = 0;
		}

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent2(ofDragInfo dragInfo);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);
		bool raySelectWithOctree(ofVec3f &pointRet);
		glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 p , glm::vec3 n);
		void ofApp::collisionResolution();
		void initThreeLighting();
		void updateLanderBounds();
		Box transformBoundingBox(const ofMatrix4x4& mat, const ofVec3f& rawMin, const ofVec3f& rawMax);
		bool playerIntersectTerrain(DynamicShape& p, TreeNode& rootNode);
		void drawLandingSquare(const glm::vec3& center, float size, ofColor color);
		bool ofApp::isInsideLandingSquare(const glm::vec3& shipPos, const glm::vec3& center, float size);
		bool isSoftLanding(const glm::vec3& velocity, float maxLandingSpeed);

		ofEasyCam cam;
		ofCamera* theCam;
		ofCamera topCam;
		ofCamera onboardCam;
		ofCamera farCam;

		ofxAssimpModelLoader mars, lander;
		vector<Octree> octrees;
		//std::vector<ofMesh> platforms;
		//std::vector<Octree> platformOctrees;

		ofLight light;
		Box boundingBox, landerBounds;
		Box testBox;
		vector<Box> colBoxList;
		bool bLanderSelected = false;
		Octree octree;
		TreeNode selectedNode;
		glm::vec3 mouseDownPos, mouseLastPos;
		bool bInDrag = false;
		ofLight keyLight, fillLight, rimLight;

		ofxIntSlider numLevels;
		ofxPanel gui;
		vector<ofColor> colors;

		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
		bool bPointSelected;
		bool bHide;
		bool pointSelected = false;
		bool bDisplayLeafNodes = false;
		bool bDisplayOctree = false;
		bool bDisplayBBoxes = false;
		
		bool bLanderLoaded;
		bool bTerrainSelected;
	
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;

		vector<Box> bboxList;

		const float selectionRange = 4.0;

		bool collided = false;
		bool resolvingCollision = false;

		// Time 
		float buildTreeTime;
		float raySearchTime;

		// Ship object
		DynamicShape spaceShip;
		int intersectAmt = 0;
		float altitude = -1;

		// Keymap and movements
		map<int, bool> keymap;
		bool movingForward;
		bool movingBackward;
		bool movingLeft;
		bool movingRight;

		// Fonts
		ofTrueTypeFont altitudeFont;
		ofTrueTypeFont gameOverFont;

		// Sound
		ofSoundPlayer explosionSound;
		ofSoundPlayer thrustSound;
	
		// background
		ofImage backgroundImg;

		// Attributes
		//glm::vec3 velocity = glm::vec3(0);     // ship's velocity
		//glm::vec3 acceleration = glm::vec3(0); // force applied (gravity, thrust)
		float gravity = -10.0f;                 // downward gravity force (adjust this)
		float maxFallSpeed = -21.0f;            // max downward speed (terminal velocity)
		bool onGround = false;                // flag for when lander is touching terrain
		bool crashDetected = false;
		float maxSafeFallSpeed = 15.0f;
		float speedIterator = 0;
		bool wasOnGround = false;


		// Fuel
		float maxFuel = 120.0f;       // 120 seconds of thrust
		float currentFuel = 120.0f;   
		bool isThrusting = false;

		// Boom
		ParticleSystem explosionSystem;
		ParticleSystem thrustParticles;

		bool explosionTriggered = false;
		bool safe = false;

		int score = 0;

		// Landing platforms
		float landingSize = 60.0f;
		glm::vec3 landingAreaCenter1 = glm::vec3(-190, 80, -190);
		glm::vec3 landingAreaCenter2 = glm::vec3(190, 40, -100); //190
		glm::vec3 landingAreaCenter3 = glm::vec3(-190, 40, 190);
		glm::vec3 landingAreaCenter4 = glm::vec3(50, 160, 100);

		// Game state
		bool bGameOver = false;
};
