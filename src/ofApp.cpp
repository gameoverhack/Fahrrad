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

	font.load(ofToDataPath("fonts/Roboto-Regular.ttf"), 96, true);

}

//--------------------------------------------------------------
void ofApp::setDefaults() {
	ofLogNotice() << className << ": setDefaults";
	nextApplicationMode = APPLICATION_NONE;
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
	case APPLICATION_STATSLOCAL:
	{
		bicycleController->update();
		pulseController->update();

		const RiderData& riderData = bicycleController->getRiderData();
		const vector<PulseData>& pulseData = pulseController->getPulseData();
		const vector<PulseData>& rawPulseData = pulseController->getRawPulseData();

		networkController->setRiderSummary(riderData.riderSummary);
		viewController->setData(riderData.riderSummary, riderData.topRiderInfo, pulseData, rawPulseData);

		networkController->update();
		viewController->update();
	}
	break;
	case APPLICATION_STATSREMOTE:
	{
		networkController->update();
		
		const RiderSummaryUnion& riderSummary = networkController->getRiderSummary();
		viewController->setData(riderSummary);

		viewController->update();
	}
	break;
	case APPLICATION_BIKEVIDEO:
	{
		bicycleController->update();
		videoController->update();
	}
	break;
	case APPLICATION_CAMERALOCAL:
	{
		imageCaptureController->update();
	}
	break;
	case APPLICATION_CAMERAREMOTE:
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
	case APPLICATION_STATSLOCAL:
	{
		
		const ofFbo& viewFbo = viewController->getFBO();
		ofPushMatrix();
		ofTranslate(viewFbo.getHeight(), 0.0f);
		ofRotateZ(90);
		viewFbo.draw(0, 0);
		ofPopMatrix();

	}
	break;
	case APPLICATION_STATSREMOTE:
	{

		const ofFbo& viewFbo = viewController->getFBO();
		viewFbo.draw(0, 0);

	}
	break;
	case APPLICATION_BIKEVIDEO:
	{
		const RiderInfo& riderInfo = bicycleController->getCurrentRiderInfo();

		float normalisedSpeed = riderInfo.normalisedSpeed;

		videoController->setSpeed(normalisedSpeed);
		videoController->getVideoTexture().draw(0, 0, ofGetWidth(), ofGetHeight());

	}
	break;
	case APPLICATION_CAMERALOCAL:
	{
		imageCaptureController->getCameraTexture().draw(0, 0, ofGetWidth() / 4, ofGetHeight() / 4);
	}
	break;
	case APPLICATION_CAMERAREMOTE:
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

		renderController->begin();
		{
			imageDisplayController->getDisplayFBO().draw(0, 0);
		}
		renderController->end();

		renderController->draw();

		imageCaptureController->getCameraTexture().draw(0, 0, ofGetWidth() / 4, ofGetHeight() / 4);
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
			case APPLICATION_STATSLOCAL:
			{
				bicycleController->drawGUI();
				pulseController->drawGUI();
				networkController->drawGUI();
				viewController->drawGUI();
			}
			break;
			case APPLICATION_STATSREMOTE:
			{
				networkController->drawGUI();
				viewController->drawGUI();
			}
			break;
			case APPLICATION_BIKEVIDEO:
			{
				bicycleController->drawGUI();
				videoController->drawGUI();
			}
			break;
			case APPLICATION_CAMERALOCAL:
			{
				imageCaptureController->drawGUI();
			}
			break;
			case APPLICATION_CAMERAREMOTE:
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
	case APPLICATION_STATSLOCAL:
	{
		if (bicycleController != nullptr) delete bicycleController;
		if (pulseController != nullptr) delete pulseController;
		if (networkController != nullptr) delete networkController;
		if (viewController != nullptr) delete viewController;
		bicycleController = nullptr;
		pulseController = nullptr;
		networkController = nullptr;
		viewController = nullptr;
	}
	break;
	case APPLICATION_STATSREMOTE:
	{
		if (networkController != nullptr) delete networkController;
		if (viewController != nullptr) delete viewController;
		networkController = nullptr;
		viewController = nullptr;
	}
	break;
	case APPLICATION_BIKEVIDEO:
	{
		if (bicycleController != nullptr) delete bicycleController;
		if (videoController != nullptr) delete videoController;
		bicycleController = nullptr;
		videoController = nullptr;
	}
	break;
	case APPLICATION_CAMERALOCAL:
	{
		if (imageCaptureController != nullptr) delete imageCaptureController;
		imageCaptureController = nullptr;
	}
	break;
	case APPLICATION_CAMERAREMOTE:
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
		if (pulseController != nullptr) delete pulseController;
		if (videoController != nullptr) delete videoController;
		if (imageCaptureController != nullptr) delete imageCaptureController;
		if (imageDisplayController != nullptr) delete imageDisplayController;
		if (renderController != nullptr) delete renderController;
		if (networkController != nullptr) delete networkController;
		if (viewController != nullptr) delete viewController;
		bicycleController = nullptr;
		pulseController = nullptr;
		videoController = nullptr;
		imageCaptureController = nullptr;
		imageDisplayController = nullptr;
		renderController = nullptr;
		networkController = nullptr;
		viewController = nullptr;
	}
	break;
	case APPLICATION_STATSLOCAL:
	{
		bicycleController = new BicycleController;
		bicycleController->setRecordRiders(true);
		bicycleController->setup();
		pulseController = new PulseController;
		pulseController->setup();
		networkController = new NetworkController;
		networkController->setup();
		networkController->setMode(NetworkController::NETWORK_SEND);
		viewController = new ViewController;
		viewController->setup();
		viewController->setMode(ViewController::VIEW_SEND);
		
	}
	break;
	case APPLICATION_STATSREMOTE:
	{
		networkController = new NetworkController;
		networkController->setup();
		networkController->setMode(NetworkController::NETWORK_RECV);
		viewController = new ViewController;
		viewController->setup();
		viewController->setMode(ViewController::VIEW_RECV);
	}
	break;
	case APPLICATION_BIKEVIDEO:
	{
		bicycleController = new BicycleController;
		bicycleController->setRecordRiders(false);
		bicycleController->setup();
		videoController = new VideoController;
		videoController->setup();
	}
	break;
	case APPLICATION_CAMERALOCAL:
	{
		imageCaptureController = new ImageCaptureController;
		imageCaptureController->setup();
	}
	break;
	case APPLICATION_CAMERAREMOTE:
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
	if (pulseController != nullptr) delete pulseController;
	if (videoController != nullptr) delete videoController;
	if (imageCaptureController != nullptr) delete imageCaptureController;
	if (imageDisplayController != nullptr) delete imageDisplayController;
	if (renderController != nullptr) delete renderController;
	if (networkController != nullptr) delete networkController;
	if (viewController != nullptr) delete viewController;
	bicycleController = nullptr;
	pulseController = nullptr;
	videoController = nullptr;
	imageCaptureController = nullptr;
	imageDisplayController = nullptr;
	renderController = nullptr;
	networkController = nullptr;
	viewController = nullptr;
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
#ifndef TARGET_WIN32
		if (!bShowFullScreen) {
			ofSetWindowShape(10, 10);
		}else{
			ofSetWindowShape(1920, 1080);
		}
#endif
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

		if(currentApplicationMode == APPLICATION_BIKEVIDEO || currentApplicationMode == APPLICATION_STATSLOCAL) bicycleController->triggerSensor(BicycleController::SENSOR_KEYBOARD);

		if (currentApplicationMode == APPLICATION_CAMERALOCAL || 
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
	if (currentApplicationMode == APPLICATION_CAMERAREMOTE ||
		currentApplicationMode == APPLICATION_DEBUG) renderController->mouseDragged(x, y);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	if (currentApplicationMode == APPLICATION_CAMERAREMOTE ||
		currentApplicationMode == APPLICATION_DEBUG) renderController->mousePressed(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	if (currentApplicationMode == APPLICATION_CAMERAREMOTE ||
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
