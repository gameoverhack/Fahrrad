#include "RenderController.h"

//--------------------------------------------------------------
RenderController::RenderController() {
	className = "RenderController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
RenderController::~RenderController() {
	ofLogNotice() << className << ": destructor";
	this->IGuiBase::~IGuiBase(); // call base destructor
}

//--------------------------------------------------------------
void RenderController::setup() {
	ofLogNotice() << className << ": setup";

	// call base clase setup for now
	IGuiBase::setup();

	//setup homography
	bSetDistortion = false;
	currentPointIndex = -1;

	originalCorners[0].set(0);
	originalCorners[1].set(ofGetWidth(), 0);
	originalCorners[2].set(ofGetWidth(), ofGetHeight());
	originalCorners[3].set(0, ofGetHeight());

	homography = ofxHomography::findHomography(originalCorners, distortedCorners);

	fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGB);

}

//--------------------------------------------------------------
void RenderController::setDefaults() {

	ofLogNotice() << className << ": setDefaults";

	distortedCorners[0].set(0, 0);
	distortedCorners[1].set(ofGetWidth(), 0);
	distortedCorners[2].set(ofGetWidth(), ofGetHeight());
	distortedCorners[3].set(0, ofGetHeight());

	homography = ofxHomography::findHomography(originalCorners, distortedCorners);

}

//--------------------------------------------------------------
void RenderController::update() {
	
}

void RenderController::draw() {
	fbo.draw(0, 0);
	if (bSetDistortion) {
		for (int i = 0; i < 4; i++) {
			ofDrawCircle(distortedCorners[i], 10);
		}
	}
}

void RenderController::begin() {
	fbo.begin();
	ofClear(0);
	ofPushMatrix();
	ofMultMatrix(homography);
}

void RenderController::end() {
	ofPopMatrix();
	fbo.end();
}

//--------------------------------------------------------------
void RenderController::drawGUI() {
	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{
			ImGui::Checkbox("Set Distortion ", &bSetDistortion);
			ImGui::Spacing();
		}
		endGUI();
	}
}

//--------------------------------------------------------------
void RenderController::mouseDragged(int x, int y) {
	
	if (currentPointIndex != -1) {
		distortedCorners[currentPointIndex] = ofPoint(x, y);
		homography = ofxHomography::findHomography(originalCorners, distortedCorners);
	}

}

//--------------------------------------------------------------
void RenderController::mousePressed(int x, int y) {

	if (bSetDistortion) {

		for (int i = 0; i < 4; i++) {

			if (distortedCorners[i].distance(ofPoint(x, y)) < 50) {
				currentPointIndex = i;
				break;
			}

		}
	}

}

//--------------------------------------------------------------
void RenderController::mouseReleased(int x, int y) {
	currentPointIndex = -1;
}

//--------------------------------------------------------------
bool RenderController::loadParameters() {
	bool ok  = Serializer.loadClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
	homography = ofxHomography::findHomography(originalCorners, distortedCorners);
	return ok;
}

//--------------------------------------------------------------
bool RenderController::saveParameters() {
	return Serializer.saveClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}