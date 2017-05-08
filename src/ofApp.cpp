#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	ofBackground(0);
	ofSetVerticalSync(false);
	ofSetFrameRate(1000);
	ofSetLogLevel(OF_LOG_NOTICE);

	className = "ApplicationController";
	// call base clase setup for now
	IGuiBase::setup();
	currentApplicationMode = APPLICATION_NONE; // so we force application mode change in update


	bShowDebug = true;
	bShowFullScreen = false;
	bShowCursor = true;

	ofSetFullscreen(bShowFullScreen);
	if (bShowCursor) {
		ofShowCursor();
	}
	else {
		ofHideCursor();
	}

	font.load(ofToDataPath("fonts/verdana.ttf"), 96, true);

}

//--------------------------------------------------------------
void ofApp::setDefaults() {
	ofLogNotice() << className << ": setDefaults";
	nextApplicationMode = APPLICATION_BIKE;
}

//--------------------------------------------------------------
void ofApp::update() {
	
	// check what mode we're in
	if (nextApplicationMode != currentApplicationMode) {
		changeMode();
	}

	switch (currentApplicationMode) {
	case APPLICATION_NONE:
	{
		// nothing for now
}
	break;
	case APPLICATION_BIKE:
	{
		bicycleController->update();
		videoController->update();
	}
	break;
	case APPLICATION_CAMERA:
	{
		imageCaptureController->update();
	}
	break;
	case APPLICATION_DISPLAY:
	{
		imageDisplayController->update();
		renderController->update();
	}
	break;
	case APPLICATION_DEBUG:
	{
		imageCaptureController->update();
		imageDisplayController->update();
		//renderController->update();
	}
	break;
	}

}

//--------------------------------------------------------------
void ofApp::draw() {

	switch (currentApplicationMode) {
	case APPLICATION_NONE:
	{
		// nothing for now
	}
	break;
	case APPLICATION_BIKE:
	{
		double avgVelocity = bicycleController->getAverageVelocity();
		double normalisedVelocity = bicycleController->getNormalisedVelocity();
		double dstTravelled = bicycleController->getDistanceTravelled();

		ostringstream os;
		os << std::setprecision(1) << std::fixed << avgVelocity << " km/h" << endl << dstTravelled << " m";

		videoController->setSpeed(normalisedVelocity);

		videoController->getVideoTexture().draw(0, 0, ofGetWidth(), ofGetHeight());
		font.drawString(os.str(), 1000, 400);

	}
	break;
	case APPLICATION_CAMERA:
	{
		imageCaptureController->getCameraTexture().draw(0, 0, ofGetWidth() / 4, ofGetHeight() / 4);
	}
	break;
	case APPLICATION_DISPLAY:
	{
		renderController->begin();
		{
			imageDisplayController->getDisplayFBO().draw(0, 0);
		}
		renderController->end();

		renderController->draw();
	}
	break;
	case APPLICATION_DEBUG:
	{
		imageCaptureController->getCameraTexture().draw(0, 0, ofGetWidth() / 4, ofGetHeight() / 4);

		renderController->begin();
		{
			imageDisplayController->getDisplayFBO().draw(0, 0);
		}
		renderController->end();

		renderController->draw();
	}
	break;
	}

	drawGUI();

}

//--------------------------------------------------------------
void ofApp::drawGUI() {

	if (bShowDebug) {
		gui.begin();
		{

			if (ImGui::CollapsingHeader(className.c_str())) {
				beginGUI();
				{
					ImGui::Combo("Application Mode", (int*)&nextApplicationMode, applicationModes);
				}
				endGUI();
			}

			switch (currentApplicationMode) {
			case APPLICATION_NONE:
			{
				// nothing for now
			}
			break;
			case APPLICATION_BIKE:
			{
				bicycleController->drawGUI();
				videoController->drawGUI();
			}
			break;
			case APPLICATION_CAMERA:
			{
				imageCaptureController->drawGUI();
			}
			break;
			case APPLICATION_DISPLAY:
			{
				imageDisplayController->drawGUI();
				renderController->drawGUI();
			}
			break;
			case APPLICATION_DEBUG:
			{
				imageCaptureController->drawGUI();
				imageDisplayController->drawGUI();
				renderController->drawGUI();
			}
			break;
			}

			ImGui::Spacing(); ImGui::Spacing();
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		
		}
		gui.end();

	}
}

//--------------------------------------------------------------
void ofApp::changeMode() {

	// shutdown the current mode

	ofLogNotice() << "Shutting down: " << applicationModes[currentApplicationMode];

	switch (currentApplicationMode) {
	case APPLICATION_NONE:
	{
		// nothing for now
	}
	break;
	case APPLICATION_BIKE:
	{
		if (bicycleController != nullptr) delete bicycleController;
		if (videoController != nullptr) delete videoController;
		bicycleController = nullptr;
		videoController = nullptr;
	}
	break;
	case APPLICATION_CAMERA:
	{
		if (imageCaptureController != nullptr) delete imageCaptureController;
		imageCaptureController = nullptr;
	}
	break;
	case APPLICATION_DISPLAY:
	{
		if (imageDisplayController != nullptr) delete imageDisplayController;
		if (renderController != nullptr) delete renderController;
		imageDisplayController = nullptr;
		renderController = nullptr;
	}
	break;
	case APPLICATION_DEBUG:
	{
		if (imageCaptureController != nullptr) delete imageCaptureController;
		if (imageDisplayController != nullptr) delete imageDisplayController;
		if (renderController != nullptr) delete renderController;
		imageCaptureController = nullptr;
		imageDisplayController = nullptr;
		renderController = nullptr;
	}
	break;
	}

	// setup for the next mode

	ofLogNotice() << "Setting up: " << applicationModes[nextApplicationMode];

	switch (nextApplicationMode) {
	case APPLICATION_NONE:
	{
		if (bicycleController != nullptr) delete bicycleController;
		if (videoController != nullptr) delete videoController;
		if (imageCaptureController != nullptr) delete imageCaptureController;
		if (imageDisplayController != nullptr) delete imageDisplayController;
		if (renderController != nullptr) delete renderController;
	}
	break;
	case APPLICATION_BIKE:
	{
		bicycleController = new BicycleController;
		bicycleController->setup();
		videoController = new VideoController;
		videoController->setup();
	}
	break;
	case APPLICATION_CAMERA:
	{
		imageCaptureController = new ImageCaptureController;
		imageCaptureController->setup();
	}
	break;
	case APPLICATION_DISPLAY:
	{
		imageDisplayController = new ImageDisplayController;
		imageDisplayController->setup();
		renderController = new RenderController;
		renderController->setup();
	}
	break;
	case APPLICATION_DEBUG:
	{
		imageCaptureController = new ImageCaptureController;
		imageCaptureController->setup();
		imageDisplayController = new ImageDisplayController;
		imageDisplayController->setup();
		renderController = new RenderController;
		renderController->setup();
	}
	break;
	}



	// set current to next mode
	currentApplicationMode = nextApplicationMode;
}

//--------------------------------------------------------------
bool ofApp::loadParameters() {
	return Serializer.loadClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool ofApp::saveParameters() {
	return Serializer.saveClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
void ofApp::exit() {
	ofLogNotice() << "Exiting Application and destroying controllers - be patient!";
	if (bicycleController != nullptr) delete bicycleController;
	if (videoController != nullptr) delete videoController;
	if (imageCaptureController != nullptr) delete imageCaptureController;
	if (imageDisplayController != nullptr) delete imageDisplayController;
	if (renderController != nullptr) delete renderController;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	switch (key) {
	case 'd':
	{
		bShowDebug ^= true;
	}
	break;
	case 'f':
	{
		bShowFullScreen ^= true;
		ofSetFullscreen(bShowFullScreen);
	}
	break;
	case 'm':
	{
		bShowCursor ^= true;
		if (bShowCursor) {
			ofShowCursor();
		}
		else {
			ofHideCursor();
		}
	}
	break;
	case ' ':
	{

		if(currentApplicationMode == APPLICATION_BIKE) bicycleController->triggerSensor(BicycleController::SENSOR_KEYBOARD);

		if (currentApplicationMode == APPLICATION_CAMERA || 
			currentApplicationMode == APPLICATION_DEBUG) imageCaptureController->triggerSensor(ImageCaptureController::SENSOR_KEYBOARD);

	}
	break;
	default:
		break;
	}

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	if (currentApplicationMode == APPLICATION_DISPLAY ||
		currentApplicationMode == APPLICATION_DEBUG) renderController->mouseDragged(x, y);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	if (currentApplicationMode == APPLICATION_DISPLAY ||
		currentApplicationMode == APPLICATION_DEBUG) renderController->mousePressed(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	if (currentApplicationMode == APPLICATION_DISPLAY ||
		currentApplicationMode == APPLICATION_DEBUG) renderController->mouseReleased(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
