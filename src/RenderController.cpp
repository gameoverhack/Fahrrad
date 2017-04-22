#include "RenderController.h"

//--------------------------------------------------------------
RenderController::RenderController() {
	className = "RenderController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
RenderController::~RenderController() {
	ofLogNotice() << className << ": destructor";

	// kill the thread
	waitForThread();

	this->IGuiBase::~IGuiBase(); // call base destructor
}

//--------------------------------------------------------------
void RenderController::setup() {
	ofLogNotice() << className << ": setup";

	// call base clase setup for now
	IGuiBase::setup();

	//setup homography
	bSetDistortion = false;

	originalCorners[0].set(0);
	originalCorners[1].set(ofGetWidth(), 0);
	originalCorners[2].set(ofGetWidth(), ofGetHeight());
	originalCorners[3].set(0, ofGetHeight());

	distortionPoints.push_back(v_0);
	distortionPoints.push_back(v_1);
	distortionPoints.push_back(v_2);
	distortionPoints.push_back(v_3);

	distortedCorners[0].set(v_0);
	distortedCorners[1].set(v_1);
	distortedCorners[2].set(v_2);
	distortedCorners[3].set(v_3);

	homography = ofxHomography::findHomography(originalCorners, distortedCorners);

	fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGB);

	imageLoadController.setup();
}

//--------------------------------------------------------------
void RenderController::setDefaults() {
	ofLogNotice() << className << ": setDefaults";

	v_0.set(0, 0);
	v_1.set(ofGetWidth(), 0);
	v_2.set(ofGetWidth(), ofGetHeight());
	v_3.set(0, ofGetHeight());
}

//--------------------------------------------------------------
void RenderController::update() {

	for (int i = 0; i < distortionPoints.size(); i++) {
		//if (distortedCorners[i] != distortionPoints[i]) {
		distortedCorners[i].set(distortionPoints[i]);
		homography = ofxHomography::findHomography(originalCorners, distortedCorners);
		//}
	}

	fbo.begin();
	{
		ofClear(0);
		ofPushMatrix();
		ofMultMatrix(homography);

		imageLoadController.getLoadedImageTexture().draw(0, 0, ofGetWidth(), ofGetHeight());

		ofPopMatrix();

		if (bSetDistortion) {
			for (int i = 0; i < distortionPoints.size(); i++) {
				ofDrawCircle(distortedCorners[i], 10);
			}
		}
	}
	fbo.end();

	imageLoadController.update();
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
	imageLoadController.drawGUI();
}

//--------------------------------------------------------------
void RenderController::mouseDragged(int x, int y) {
	if (movingPoint) {
		curPoint->set(x, y);
	}
}

//--------------------------------------------------------------
void RenderController::mousePressed(int x, int y) {
	if (bSetDistortion) {
		ofPoint cur(x, y);

		for (int i = 0; i < distortionPoints.size(); i++) {
			if (distortionPoints[i].distance(cur) < 50) {
				movingPoint = true;
				curPoint = &distortionPoints[i];
			}
		}
	}
}

//--------------------------------------------------------------
void RenderController::mouseReleased(int x, int y) {
	movingPoint = false;
}

//--------------------------------------------------------------
ofTexture & RenderController::getTexture() {
	return fbo.getTextureReference(0);
}

//--------------------------------------------------------------
bool RenderController::loadParameters() {
	return Serializer.loadClass(fixPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool RenderController::saveParameters() {
	return Serializer.saveClass(fixPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}