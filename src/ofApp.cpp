#include "ofApp.h"
#include "Util.h"

//
// Some setup code and functions reused from the starter code for 3D 
// interaction Lab and spacial subdivision written by Kevin M. Smith for CS134 
// 
// 
// The rest of the function code is written by Danilo Makarewycz and Nathan Wong
// For the CS134 Final project
//
//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
//	ofSetWindowShape(1024, 768);
	cam.setDistance(10);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	ofSetVerticalSync(true);
	//cam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();

	// lighting 
	//initLightingAndMaterials();
	initThreeLighting();

	//mars.loadModel("geo/platformsPlusTerrain.obj");
	mars.loadModel("geo/finalTerrain.obj");
	mars.setScaleNormalization(false);

	// create sliders for testing
	//
	//gui.setup();
	//gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	
	//Set this to false later
	//bDisplayOctree = false;

	// Font
	altitudeFont.load("fonts/Play-Regular.ttf", 16); 

	// Sound
	explosionSound.load("sounds/projectile_explosion.mp3");
	thrustSound.load("sounds/rocket_thruster.wav");
	thrustSound.setVolume(0.90f);
	thrustSound.setMultiPlay(false);
	thrustSound.setLoop(false);

	bHide = false;
	
	// Color Vector
	colors = {
		//ofColor::red,
		ofColor::white,
		ofColor::green,
		ofColor::blue,
		ofColor::orange,
		ofColor::yellow,
		ofColor::purple,
		ofColor::white,
		ofColor::cyan,
		ofColor::teal,
		ofColor::magenta
	};

	float startTime = ofGetElapsedTimeMillis();
	octree.create(mars.getMesh(0), 20);
	float endTime = ofGetElapsedTimeMillis();

	/*for (int i = 2; i < 7; i++) { 
		platforms.push_back(mars.getMesh(i));
	}*/


	buildTreeTime = endTime - startTime;
	//cout << "Octree build time: " << buildTreeTime << " ms" << endl;

	//cout << "Number of Verts: " << mars.getMesh(0).getNumVertices() << endl;

	if (lander.loadModel("geo/shipTest5.obj")) {
		bLanderLoaded = true;
		lander.setScaleNormalization(false);
		lander.setPosition(0, 500, -250);

		// scale
		float scale = 0.01f;
		lander.setScale(scale, scale, scale);

		//lander.setRotation(0, 180, 0, 1, 0);
		//spaceShip.angle += 180;

		bboxList.clear();
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		// Initialize ship physics object at lander position
		spaceShip.position = lander.getPosition();
	}
	else {
		ofLogError() << "Could not load lander model.";
	}

	topCam.lookAt(glm::vec3(0, -1, 0));
	topCam.setPosition(spaceShip.position.x, spaceShip.position.y + 70, spaceShip.position.z);
	topCam.setNearClip(.1);
	onboardCam.lookAt(glm::vec3(0, 0, -3));
	onboardCam.setPosition(spaceShip.position.x, spaceShip.position.y, spaceShip.position.z);
	onboardCam.setFov(65);
	farCam.lookAt(glm::vec3(0, -1, 0));
	farCam.setPosition(-715, 80, -800);
	farCam.setNearClip(.1);

	glm::vec3 offset(80, 40, 100);  // (x: right, y: above, z: behind)
	glm::vec3 camPos = spaceShip.position + offset;

	cam.setPosition(camPos);
	cam.lookAt(spaceShip.position);

	theCam = &cam;

	backgroundImg.load("images/Starry_Sky.png"); 

	explosionSystem.setPosition(ofVec3f(0, 0, 0));

	thrustParticles.setPosition(spaceShip.position);

}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {

	topCam.setPosition(spaceShip.position.x, spaceShip.position.y + 70, spaceShip.position.z);
	onboardCam.setPosition(spaceShip.position.x, spaceShip.position.y, spaceShip.position.z - 6);
	farCam.lookAt(spaceShip.position);
	
	if (bGameOver) {
		spaceShip.velocity = glm::vec3(0, 0, 0);
	}

	// Fuel
	if (isThrusting && currentFuel > 0) {
		thrustSound.play();
		float dt = 1.0f / ofGetFrameRate();  // time per frame
		currentFuel -= dt;

		if (currentFuel <= 0) {
			currentFuel = 0;
			isThrusting = false;
		}
	}

	// Boom particles
	float dt = 1.0f / ofGetFrameRate();
	explosionSystem.update(dt);

	if (crashDetected && !explosionTriggered) {
		explosionTriggered = true;
		explosionSystem.setPosition(spaceShip.position);
		explosionSystem.start(350);  // 350 particles
	}

	// Thrust particles
	thrustParticles.setPosition(spaceShip.position + ofVec3f(0, -3, 0));  // sligthly under lander
	thrustParticles.update(dt);

	// Control thrust particle emission
	if (isThrusting && currentFuel > 0) {
		thrustParticles.startContinuous();
	}
	else {
		thrustParticles.stop();
	}


	if (colBoxList.size() >= 10) {
		onGround = true;
		collided = true;
	}
	if (colBoxList.size() < 10) {
		collided = false;
	}

	/*if ((collided || onGround) && !crashDetected && !safe) {
		if (spaceShip.velocity.y < -maxSafeFallSpeed) {
			crashDetected = true;
			cout << "You have crashed!" << endl;
		}
		else {
			safe = true;
			cout << "Safe landing!" << endl;
		}
	}*/

	if (collided) {
		//cout << "Collided " << intersectAmt++ << endl;
		resolvingCollision = true;
	}

	// Boom
	if (crashDetected && !explosionTriggered && !safe) {
		explosionTriggered = true;
		explosionSystem.setPosition(spaceShip.position);
		explosionSystem.start();
		lander.setScale(0, 0, 0); // hide lander
	}

	// Landing
	if (collided) {

		bool landedInside1 = isInsideLandingSquare(spaceShip.position, landingAreaCenter1, landingSize);
		bool landedInside2 = isInsideLandingSquare(spaceShip.position, landingAreaCenter2, landingSize);
		bool landedInside3 = isInsideLandingSquare(spaceShip.position, landingAreaCenter3, landingSize);
		bool landedInside4 = isInsideLandingSquare(spaceShip.position, landingAreaCenter4, landingSize);

		bool insideAnyLandingZone = landedInside1 || landedInside2 || landedInside3 || landedInside4;
		bool landedSoftly = isSoftLanding(spaceShip.velocity, maxSafeFallSpeed);

		bool bLanderLandedSafely = insideAnyLandingZone && landedSoftly;

		if (bLanderLandedSafely) {
			//cout << "Safe landing in one of the zones!" << endl;
			score = 1000;  // max score
			spaceShip.velocity = glm::vec3(0, 0, 0);
			bGameOver = true;
		}
		else {
			//cout << "Landing failed!" << endl;
			crashDetected = true;
			explosionSound.play();
			lander.setScale(0, 0, 0);
			bGameOver = true;
		}

		// Stop movement after collision
		spaceShip.velocity = glm::vec3(0);
		spaceShip.acceleration = glm::vec3(0);
	}



	// Altitude Sensor
	ofVec3f landerPos = lander.getPosition();
	ofVec3f rayOrigin = landerPos;
	ofVec3f rayDirection = ofVec3f(0, -1, 0);
	Ray downRay = Ray(Vector3(rayOrigin.x, rayOrigin.y, rayOrigin.z),
		Vector3(rayDirection.x, rayDirection.y, rayDirection.z));

	TreeNode altitudeNode;
	bool hit = octree.intersect(downRay, octree.root, altitudeNode);

	//altitude = -1;
	if (hit && !altitudeNode.points.empty()) {
		int closestIndex = altitudeNode.points[0];
		ofVec3f terrainPoint = octree.mesh.getVertex(closestIndex);
		altitude = landerPos.y - terrainPoint.y;
		//cout << "Altitude: " << altitude << endl;
	}

	/*if (altitude <= 5) {
		onGround = true;
	}
	else {
		onGround = false;
	}*/

	if (bLanderLoaded) {
		lander.setRotation(0, spaceShip.angle, 0, 1, 0); //index, angle (degrees), x axis rotation, y axis rotation, z axis rotation
		lander.setPosition(spaceShip.position.x, spaceShip.position.y, spaceShip.position.z);
		if (!onGround) {
			spaceShip.forces += glm::vec3(0, gravity, 0);
		}
		spaceShip.integrate();
	}

	if (resolvingCollision) {
		collisionResolution();
	}

	if ((onGround || collided) && !crashDetected) {
		spaceShip.velocity.y = abs(spaceShip.velocity.y) * 0.8;
	}

	if (!wasOnGround && (onGround || collided) && !crashDetected) {
		wasOnGround = true;
		safe = true;
	}

	/*if (onGround) {
		spaceShip.velocity.y = 0;
	}*/

	if (explosionTriggered) {
		spaceShip.velocity.y = 0;
	}


	// Testing downward spd
	//if (spaceShip.velocity.y < -maxSafeFallSpeed) {
	//	cout << "Speed Warning! " << spaceShip.velocity.y << endl;//speedIterator++<< endl;
	//}
	//cout << "Speed: " << spaceShip.velocity.y << endl;//speedIterator++<< endl;

	// Keymap
	if (keymap[OF_KEY_UP] || keymap['W'] || keymap['w']) {
		if (bLanderLoaded) {
			spaceShip.moveForward();
		}
	}
	if (keymap[OF_KEY_DOWN] || keymap['S'] || keymap['s']) {
		if (bLanderLoaded) {
			spaceShip.moveBackward();
		}
	}
	if (keymap[OF_KEY_RIGHT] || keymap['D'] || keymap['d']) {
		if (bLanderLoaded) {
			//spaceShip.angle -= 5;
			spaceShip.turnRight();
			//topCam.rotate(-0.5, 0, 1, 0);
		}
	}
	if (keymap[OF_KEY_LEFT] || keymap['A'] || keymap['a']) {
		if (bLanderLoaded) {
			//spaceShip.angle += 5;
			spaceShip.turnLeft();
			//topCam.rotate(0.5, 0, 1, 0);
		}
	}

	if (currentFuel > 0) {
		if (keymap[' ']) {
			if (bLanderLoaded) {
				spaceShip.moveUp();
				isThrusting = true;
			}
		}
	}
	
	if (keymap['X'] || keymap['x']) {
		if (bLanderLoaded) {
			spaceShip.moveDown();
		}
	}



	// Intersection Testing 
	// Get the local-space min/max of the lander
	ofVec3f rawMin = lander.getSceneMin();
	ofVec3f rawMax = lander.getSceneMax();
	ofMatrix4x4 mat = lander.getModelMatrix();

	// Generate all 8 corners of the bounding box
	std::vector<ofVec3f> corners;
	corners.push_back(mat.preMult(ofVec3f(rawMin.x, rawMin.y, rawMin.z)));
	corners.push_back(mat.preMult(ofVec3f(rawMin.x, rawMin.y, rawMax.z)));
	corners.push_back(mat.preMult(ofVec3f(rawMin.x, rawMax.y, rawMin.z)));
	corners.push_back(mat.preMult(ofVec3f(rawMin.x, rawMax.y, rawMax.z)));
	corners.push_back(mat.preMult(ofVec3f(rawMax.x, rawMin.y, rawMin.z)));
	corners.push_back(mat.preMult(ofVec3f(rawMax.x, rawMin.y, rawMax.z)));
	corners.push_back(mat.preMult(ofVec3f(rawMax.x, rawMax.y, rawMin.z)));
	corners.push_back(mat.preMult(ofVec3f(rawMax.x, rawMax.y, rawMax.z)));

	// Initialize new min and max to the first corner
	float minX = corners[0].x, minY = corners[0].y, minZ = corners[0].z;
	float maxX = corners[0].x, maxY = corners[0].y, maxZ = corners[0].z;

	// Find min and max across all transformed corners
	for (int i = 1; i < corners.size(); ++i) {
		minX = std::min(minX, corners[i].x);
		minY = std::min(minY, corners[i].y);
		minZ = std::min(minZ, corners[i].z);
		maxX = std::max(maxX, corners[i].x);
		maxY = std::max(maxY, corners[i].y);
		maxZ = std::max(maxZ, corners[i].z);
	}

	// Assign to landerBounds
	landerBounds = Box(Vector3(minX, minY, minZ), Vector3(maxX, maxY, maxZ));
	
	colBoxList.clear();
	octree.intersect(landerBounds, octree.root, colBoxList);

	topCam.rotate(spaceShip.torque, 0, 1, 0);
	onboardCam.rotate(spaceShip.torque, 0, 1, 0);

}
//--------------------------------------------------------------
void ofApp::draw() {

	//ofBackground(ofColor::black);
	ofDisableDepthTest();
	backgroundImg.draw(0, 0, ofGetWidth(), ofGetHeight());
	ofEnableDepthTest();

	//glDepthMask(false);
	//if (!bHide) gui.draw();
	//glDepthMask(true);

	theCam->begin();

	// Platform 1
	drawLandingSquare(landingAreaCenter1, landingSize, ofColor::blue);

	// Platform 2
	drawLandingSquare(landingAreaCenter2, landingSize, ofColor::green);

	// Platform 3
	drawLandingSquare(landingAreaCenter3, landingSize, ofColor::red);

	// Platform 4
	drawLandingSquare(landingAreaCenter4, landingSize, ofColor::orange);

	explosionSystem.draw();
	thrustParticles.draw();

	ofPushMatrix();
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
		if (bLanderLoaded) {
			lander.drawWireframe();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();
		ofMesh mesh;
		if (bLanderLoaded) {
			lander.drawFaces();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
			if (bDisplayBBoxes) {
				ofNoFill();
				ofSetColor(ofColor::white);
				for (int i = 0; i < lander.getNumMeshes(); i++) {
					ofPushMatrix();
					ofMultMatrix(lander.getModelMatrix());
					ofRotate(-90, 1, 0, 0);
					updateLanderBounds(); 
					Octree::drawBox(bboxList[i]);  //box
					ofPopMatrix();
				}
			}

			//updateLanderBounds();
			//Octree::drawBox(landerBounds);

			if (bLanderSelected) {

				//spaceShip.drawLine();
				//spaceShip.drawDownLine();  // heading that points down
		

				ofVec3f min = lander.getSceneMin() + lander.getPosition();
				ofVec3f max = lander.getSceneMax() + lander.getPosition();

				Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
				ofSetColor(ofColor::white);

				// The bounding box
				//Octree::drawBox(bounds);

				// draw colliding boxes
				//
				//ofSetColor(ofColor::lightBlue);
				for (int i = 0; i < colBoxList.size(); i++) {
					Octree::drawBox(colBoxList[i]);   // for collision 
				}
			}
		}
	}
	if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));

	// Draw platforms
	//for (auto& mesh : platforms) {
	//	mesh.drawFaces(); //mesh.draw();
	//}

	for (int i = 2; i <= 6; i++) {
		if (i < mars.getMeshCount()) {
			ofMesh mesh = mars.getMesh(i);
			mesh.draw();  
		}
	}

	if (bDisplayPoints) {               
		glPointSize(3);
		//ofSetColor(ofColor::green);
		mars.drawVertices();
	}

	// recursively draw octree
	//
	ofDisableLighting();
	int level = 0;
	//	ofNoFill();

	if (bDisplayLeafNodes) {
		//ofSetColor(colours);  // Set color
		octree.numLeaf = 0; 
		octree.drawLeafNodes(octree.root);
		cout << "num leaf: " << octree.numLeaf << endl;
    }
	else if (bDisplayOctree) {
		ofNoFill();
		ofSetColor(ofColor::white);
		octree.draw(1, 0, colors);
	}

	// if point selected, draw a sphere
	//
	if (pointSelected) {
		ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
		ofVec3f d = p - cam.getPosition();
		ofSetColor(ofColor::lightGreen);
		ofDrawSphere(p, .02 * d.length());
		cout << "Click Coordinate: " << p << endl;
	}

	ofPopMatrix();
	theCam->end();


	// Fuel Reading
	ofDisableDepthTest();
	ofDrawBitmapStringHighlight("Fuel Remaining: " + ofToString(currentFuel, 1) + " s", 10, 20);

	// Draw altitude reading
	ofSetColor(ofColor::white);
	std::string altText = "Altitude: " + ofToString(altitude, 2) + " feet";
	float textWidth = altitudeFont.stringWidth(altText);
	float textHeight = altitudeFont.stringHeight(altText);
	float x = ofGetWidth() - textWidth - 20;
	float y = textHeight + 20;
	if (altitude > -1) {
		altitudeFont.drawString(altText, x, y);
	}
	else {
		altitudeFont.drawString("Altitude: Danger!", x, y);
	}

	string scoreText = "Score: " + std::to_string(score);
	ofDrawBitmapStringHighlight(scoreText, ofGetWidth() / 2 - 40, 20);  // Top middle 

	string controlsText = "WASD for Movement\nX to go down\nSpace to go up\nF1-F5 for cameras\n r to reset game";
	// [Controls\ Bottom-left corner
	float x_con = 20; 
	float y_con = ofGetHeight() - 60;  
	ofSetColor(255);  // white color
	ofDrawBitmapString(controlsText, x_con, y_con);

	if (bGameOver) {
		ofDisableDepthTest();
		gameOverFont.load("fonts/Play-Bold.ttf", 32);

		// Dim background
		ofFill();
		ofSetColor(0, 0, 0, 150);
		ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
		ofNoFill();

		// Set lines
		std::string line1 = "Game Over";
		std::string line2 = "Score: " + std::to_string(score);
		std::string line3 = "Press 'r' to Restart";

		// Get bounding boxes
		ofRectangle bounds1 = gameOverFont.getStringBoundingBox(line1, 0, 0);
		ofRectangle bounds2 = gameOverFont.getStringBoundingBox(line2, 0, 0);
		ofRectangle bounds3 = gameOverFont.getStringBoundingBox(line3, 0, 0);

		// Calculate vertical positions
		float centerY = ofGetHeight() / 2;
		float spacing = 15;
		float totalHeight = bounds1.getHeight() + bounds2.getHeight() + bounds3.getHeight() + spacing * 2;

		float yStart = centerY - totalHeight / 2;

		// Draw lines centered
		ofSetColor(ofColor::white);
		gameOverFont.drawString(line1, ofGetWidth() / 2 - bounds1.getWidth() / 2, yStart + bounds1.getHeight());
		gameOverFont.drawString(line2, ofGetWidth() / 2 - bounds2.getWidth() / 2, yStart + bounds1.getHeight() + spacing + bounds2.getHeight());
		gameOverFont.drawString(line3, ofGetWidth() / 2 - bounds3.getWidth() / 2, yStart + bounds1.getHeight() + bounds2.getHeight() + spacing * 2 + bounds3.getHeight());
	}


}


// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {

	if (isalpha(key)) {  // Only apply tolower() for letters
		keymap[tolower(key)] = true;
	}
	else {
		keymap[key] = true;
	}

	switch (key) {
	case 'B':
	case 'b':
		bDisplayBBoxes = !bDisplayBBoxes;
		break;
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'L':
	case 'l': //
		bDisplayLeafNodes = !bDisplayLeafNodes;
		break;
	case 'O':
	case 'o':
		bDisplayOctree = !bDisplayOctree;
		break;
	case 'R':
	case 'r':
		restartGame();
		break;
	case '=':
		savePicture();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'm':
		toggleWireframeMode();
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	case 'j':
		if (collided) {
			resolvingCollision = true;
		}
		break;
	case 'p':
		cout << "Spaceship pos: " << spaceShip.position << endl;
		break;

	case OF_KEY_F1:
		theCam = &cam;
		break;
	case OF_KEY_F2:
		theCam = &topCam;
		break;
	case OF_KEY_F3:
		theCam = &onboardCam;
		break;
	case OF_KEY_F4:
		theCam = &farCam;
		break;
	case OF_KEY_F5:
		cam.lookAt(spaceShip.position);
	default:
		break;
	}
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	if (isalpha(key)) {  // Only apply tolower() for letters
		keymap[tolower(key)] = false;
	}
	else {
		keymap[key] = false;
	}

	if (!keymap[OF_KEY_UP] || !keymap['W'] || !keymap['w']) {
		//isThrusting = false;
	}
	if (!keymap[OF_KEY_DOWN] || !keymap['S'] || !keymap['s']) {
		//isThrusting = false;
	}
	if (!keymap[' ']) {
		isThrusting = false;
	}

	switch (key) {
	
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	default:
		break;

	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	// if moving camera, don't allow mouse interaction
//
	if (cam.getMouseInputEnabled()) return;

	// if rover is loaded, test for selection
	//
	if (bLanderLoaded) {
		glm::vec3 origin = cam.getPosition();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = landerBounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			mouseDownPos = getMousePointOnPlane(lander.getPosition(), cam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			bLanderSelected = false;
		}
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

bool ofApp::raySelectWithOctree(ofVec3f &pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	pointSelected = octree.intersect(ray, octree.root, selectedNode);

	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
	}
	return pointSelected;
}




//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	if (bInDrag) {

		glm::vec3 landerPos = lander.getPosition();

		glm::vec3 mousePos = getMousePointOnPlane(landerPos, cam.getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;
	
		landerPos += delta;
		//lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
		spaceShip.position = glm::vec3(landerPos.x, landerPos.y, landerPos.z);
		mouseLastPos = mousePos;

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(landerBounds, octree.root, colBoxList);
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent2(ofDragInfo dragInfo) {

	ofVec3f point;
	mouseIntersectPlane(ofVec3f(0, 0, 0), cam.getZAxis(), point);
	if (lander.loadModel(dragInfo.files[0])) {
		lander.setScaleNormalization(false);
		lander.setScale(0.05, 0.05, 0.05);
//		lander.setScale(.1, .1, .1);
	//	lander.setPosition(point.x, point.y, point.z);
		lander.setPosition(1, 1, 0);

		bLanderLoaded = true;
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		cout << "Mesh Count: " << lander.getMeshCount() << endl;
	}
	else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (lander.loadModel(dragInfo.files[0])) {
		bLanderLoaded = true;
		lander.setScaleNormalization(false);
		lander.setPosition(0, 0, 0);
		lander.setScale(0.05, 0.05, 0.05);
		cout << "number of meshes: " << lander.getNumMeshes() << endl;
		bboxList.clear();
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		glm::vec3 origin = cam.getPosition();
		glm::vec3 camAxis = cam.getZAxis();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the lander's origin at that intersection point
			//
			glm::vec3 min = lander.getSceneMin();
			glm::vec3 max = lander.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			lander.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for lander while we are at it
			//
			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}


}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = cam.getPosition();
	glm::vec3 camAxis = cam.getZAxis();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}

// Moves lander in opposite direction during collision
void ofApp::collisionResolution() {
	if (!bLanderLoaded) {
		return;
	}

	ofVec3f landPos = lander.getPosition();
	spaceShip.position = glm::vec3(landPos.x, landPos.y + 0.05, landPos.z);

	colBoxList.clear();
	ofVec3f min = lander.getSceneMin() + lander.getPosition();
	ofVec3f max = lander.getSceneMax() + lander.getPosition();
	Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
	octree.intersect(boundingBox, octree.root, colBoxList);

	if (colBoxList.size() < 10) {
		collided = false;
		resolvingCollision = false;
	}

}

void ofApp::initThreeLighting() {
	keyLight.setup();
	keyLight.enable();
	keyLight.setDirectional();
	keyLight.setAmbientColor(ofFloatColor(0.05, 0.05, 0.05));
	keyLight.setDiffuseColor(ofFloatColor(1.0, 0.95, 0.9));
	keyLight.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0));
	keyLight.setOrientation(ofVec3f(45, -60, 0));

	fillLight.setup();
	fillLight.enable();
	fillLight.setSpotlight();
	fillLight.setScale(0.1);
	fillLight.setSpotlightCutOff(35);
	fillLight.setAttenuation(1.5, 0.002, 0.001);
	fillLight.setAmbientColor(ofFloatColor(0.05, 0.05, 0.05));
	fillLight.setDiffuseColor(ofFloatColor(0.7, 0.8, 1.0));
	fillLight.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0));
	fillLight.setOrientation(ofVec3f(20, 45, 0));
	fillLight.setPosition(-400, 300, 400);

	rimLight.setup();
	rimLight.enable();
	rimLight.setSpotlight();
	rimLight.setScale(0.1);
	rimLight.setSpotlightCutOff(40);
	rimLight.setAttenuation(1.0, 0.002, 0.001);
	rimLight.setAmbientColor(ofFloatColor(0.05, 0.05, 0.05));
	rimLight.setDiffuseColor(ofFloatColor(1.0, 1.0, 1.0));
	rimLight.setSpecularColor(ofFloatColor(1.0, 1.0, 1.0));
	rimLight.setOrientation(ofVec3f(-45, 180, 0));
	rimLight.setPosition(0, 250, -500);
}



void ofApp::updateLanderBounds() {
	ofVec3f rawMin = lander.getSceneMin();
	ofVec3f rawMax = lander.getSceneMax();
	ofMatrix4x4 mat = lander.getModelMatrix();

	ofVec3f worldMin = mat.preMult(rawMin);
	ofVec3f worldMax = mat.preMult(rawMax);

	glm::vec3 center = (worldMin + worldMax) / 2.0f;
	glm::vec3 halfSize = (worldMax - worldMin) / 2.0f * 0.5f; 

	glm::vec3 newMin = center - halfSize;
	glm::vec3 newMax = center + halfSize;

	landerBounds = Box(Vector3(newMin.x, newMin.y, newMin.z),
		Vector3(newMax.x, newMax.y, newMax.z));
}

void ofApp::drawLandingSquare(const glm::vec3& center, float size, ofColor color) {
	float halfSize = size / 2.0f;
	float y = center.y;  // fixed height

	glm::vec3 p1(center.x - halfSize, y, center.z - halfSize);
	glm::vec3 p2(center.x + halfSize, y, center.z - halfSize);
	glm::vec3 p3(center.x + halfSize, y, center.z + halfSize);
	glm::vec3 p4(center.x - halfSize, y, center.z + halfSize);

	//ofSetColor(ofColor::blue);
	//ofNoFill();
	ofFill();
	ofSetColor(color);

	ofDrawLine(p1, p2);
	ofDrawLine(p2, p3);
	ofDrawLine(p3, p4);
	ofDrawLine(p4, p1);
}

bool ofApp::isInsideLandingSquare(const glm::vec3& shipPos, const glm::vec3& center, float size) {
	float halfSize = size / 2.0f;

	// Check if ship's x,z are within square bounds
	bool insideX = (shipPos.x >= center.x - halfSize) && (shipPos.x <= center.x + halfSize);
	bool insideZ = (shipPos.z >= center.z - halfSize) && (shipPos.z <= center.z + halfSize);

	return insideX && insideZ;
}

bool ofApp::isSoftLanding(const glm::vec3& velocity, float maxLandingSpeed) {
	// For soft landing, vertical speed should be less than maxLandingSpeed
	return glm::abs(velocity.y) <= maxLandingSpeed;
}

