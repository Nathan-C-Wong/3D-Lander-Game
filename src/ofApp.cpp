#include "ofApp.h"
#include "Util.h"

//
// Some setup code and functions reused from the starter code for 3D 
// interaction Lab from Kevin M. Smith's SJSU CS134 Class
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
	initLightingAndMaterials();
	//initThreeLighting();

	mars.loadModel("geo/terrainTest9.obj");
	mars.setScaleNormalization(false);

	// create sliders for testing
	//
	//gui.setup();
	//gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	
	//Set this to false later
	//bDisplayOctree = false;

	// Font
	altitudeFont.load("fonts/Play-Regular.ttf", 16); 


	bHide = false;
	
	// Color Vector
	colors = {
		ofColor::red,
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

	buildTreeTime = endTime - startTime;
	cout << "Octree build time: " << buildTreeTime << " ms" << endl;

	//cout << "Number of Verts: " << mars.getMesh(0).getNumVertices() << endl;

	//testBox = Box(Vector3(3, 3, 0), Vector3(5, 5, 2));


	//vector<Octree> octrees; 
	//octrees.clear();
	//for (int i = 2; i <= 6; i++) {
	//	if (i < mars.getMeshCount()) {
	//		ofMesh mesh = mars.getMesh(i);
	//		Octree newOctree;
	//		newOctree.create(mesh, 20);  // build octree for this mesh
	//		octrees.push_back(newOctree);

	//		//cout << "Built octree for mesh " << i << " with " << mesh.getNumVertices() << " vertices." << endl;
	//	}
	//	else {
	//		//cout << "Mesh index " << i << " out of range." << endl;
	//	}
	//}


	if (lander.loadModel("geo/shipTest5.obj")) {
		bLanderLoaded = true;
		lander.setScaleNormalization(false);
		lander.setPosition(0, 500, 0);

		// scale
		float scale = 0.01f;
		lander.setScale(scale, scale, scale);

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
	onboardCam.lookAt(glm::vec3(0, 0, -1));
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
}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {

	topCam.setPosition(spaceShip.position.x, spaceShip.position.y + 70, spaceShip.position.z);
	onboardCam.setPosition(spaceShip.position.x, spaceShip.position.y, spaceShip.position.z - 6);
	farCam.lookAt(spaceShip.position);

	// Fuel
	if (isThrusting && currentFuel > 0) {
		float dt = 1.0f / ofGetFrameRate();  // time per frame
		currentFuel -= dt;

		if (currentFuel <= 0) {
			currentFuel = 0;
			// Optionally disable thrusting
			isThrusting = false;
		}
	}

	if (colBoxList.size() >= 10) {
		collided = true;
	}
	if (colBoxList.size() < 10) {
		collided = false;
	}

	if (collided) {
		cout << "Collided " << intersectAmt++ << endl;
		resolvingCollision = true;
	}

	if (resolvingCollision) {
		collisionResolution();
	}

	/*if (altitude < 4) {
		cout << "On ground" << endl;
		onGround = true;
	}
	else {
		onGround = false;
	}*/

	if (bLanderLoaded) {
		lander.setRotation(0, spaceShip.angle, 0, 1, 0); //index, angle (degrees), x axis rotation, y axis rotation, z axis rotation
		lander.setPosition(spaceShip.position.x, spaceShip.position.y, spaceShip.position.z);
		if (!collided) {
			spaceShip.forces += glm::vec3(0, gravity, 0);
		}
		spaceShip.integrate();
	}

	

	// After velocity update:
	if ((velocity.y < -maxSafeFallSpeed) && collided) {
		// Maybe trigger a crash if landed
		crashDetected = true;
	}
	if (crashDetected) {
		cout << "You have crashed!" << endl;
	}


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
	// Get the raw (local-space) min/max of the lander
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

	topCam.rotate(spaceShip.torque, 0, 1, 0);
	onboardCam.rotate(spaceShip.torque, 0, 1, 0);

}
//--------------------------------------------------------------
void ofApp::draw() {

	//ofBackground(ofColor::black);
	ofDisableDepthTest();
	backgroundImg.draw(0, 0, ofGetWidth(), ofGetHeight());
	ofDrawBitmapStringHighlight("Fuel Remaining: " + ofToString(currentFuel, 1) + " s", 10, 20);
	ofEnableDepthTest(); 


	// Draw altitude reading
	ofSetColor(ofColor::white);
	std::string altText = "Altitude: " + ofToString(altitude, 2) + " m";
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
	

	//glDepthMask(false);
	//if (!bHide) gui.draw();
	//glDepthMask(true);

	theCam->begin();
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
					//Octree::drawBox(bboxList[i]);
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



	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
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
		//ofNoFill();
		//ofSetColor(ofColor::white);
		//octree.draw(numLevels, 0, colors);
	}

	// if point selected, draw a sphere
	//
	if (pointSelected) {
		ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
		ofVec3f d = p - cam.getPosition();
		//ofSetColor(ofColor::lightGreen);
		//ofDrawSphere(p, .02 * d.length());
		//cout << "Click Coordinate: " << p << endl;
	}

	ofPopMatrix();
	theCam->end();

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
	case 'r':
		cam.reset();
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
	spaceShip.position = glm::vec3(landPos.x, landPos.y + 0.1, landPos.z);

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
	keyLight.setAreaLight(1, 1);
	keyLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	keyLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	keyLight.setSpecularColor(ofFloatColor(1, 1, 1));

	keyLight.rotate(45, ofVec3f(0, 1, 0));
	keyLight.rotate(-45, ofVec3f(1, 0, 0));
	keyLight.setPosition(500, 5, 500);

	fillLight.setup();
	fillLight.enable();
	fillLight.setSpotlight();
	fillLight.setScale(.05);
	fillLight.setSpotlightCutOff(15);
	fillLight.setAttenuation(2, .001, .001);
	fillLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	fillLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	fillLight.setSpecularColor(ofFloatColor(1, 1, 1));
	fillLight.rotate(-10, ofVec3f(1, 0, 0));
	fillLight.rotate(-45, ofVec3f(0, 1, 0));
	fillLight.setPosition(-500, 5, 500);

	rimLight.setup();
	rimLight.enable();
	rimLight.setSpotlight();
	rimLight.setScale(.05);
	rimLight.setSpotlightCutOff(30);
	rimLight.setAttenuation(.2, .001, .001);
	rimLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	rimLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	rimLight.setSpecularColor(ofFloatColor(1, 1, 1));
	rimLight.rotate(180, ofVec3f(0, 1, 0));
	rimLight.setPosition(0, 150, -7);
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

//bool ofApp::playerIntersectTerrain(DynamicShape& p, TreeNode& rootNode) {
//	glm::vec3 halfSize = glm::vec3(p.playerBox.width, p.playerBox.height, p.playerBox.depth) * 0.5f;
//	vector<glm::vec3> localCorners;
//
//	for (int x = -1; x <= 1; x += 2) {
//		for (int y = -1; y <= 1; y += 2) {
//			for (int z = -1; z <= 1; z += 2) {
//				glm::vec3 corner = glm::vec3(x * halfSize.x, y * halfSize.y, z * halfSize.z);        //finding the 8 corners of player's box
//				localCorners.push_back(corner);
//			}
//		}
//	}
//	glm::mat4 transform = p.playerBox.getTransform();
//	vector<glm::vec3> worldCorners;
//	for (int i = 0; i < localCorners.size(); i++) {
//		glm::vec4 world = transform * glm::vec4(localCorners[i], 1.0);
//		worldCorners.push_back(glm::vec3(world));                //getting 8 corners in world space
//	}
//	float minY = worldCorners[0].y;
//	for (int i = 0; i < worldCorners.size(); i++) {
//		minY = std::min(minY, worldCorners[i].y);
//	}
//	for (int i = 0; i < worldCorners.size(); i++) {
//		if (abs(worldCorners[i].y - minY) < 0.01f) {  // tolerance on bottom corners
//			TreeNode hitNode;
//			if (octree.intersect(worldCorners[i], octree.root, hitNode)) {
//				return true;
//			}
//		}
//	}
//
//	return false;
//}