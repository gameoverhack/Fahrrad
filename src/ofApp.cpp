#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0);
	ofSetVerticalSync(false);
	ofSetFrameRate(1000);
	ofSetLogLevel(OF_LOG_VERBOSE);

	bShowDebug = true;
	bShowFullScreen = false;
	bShowCursor = true;

	ofSetFullscreen(bShowFullScreen);
	if (bShowCursor) {
		ofShowCursor();
	} else {
		ofHideCursor();
	}

	bicycleController.setup();
	videoController.setup();

	font.load(ofToDataPath("fonts/verdana.ttf"), 96, true);
}

//--------------------------------------------------------------
void ofApp::update(){
	
	bicycleController.update();
	videoController.update();

}

//--------------------------------------------------------------
void ofApp::draw(){

	double avgVelocity = bicycleController.getAverageVelocity();
	double normalisedVelocity = bicycleController.getNormalisedVelocity();
	double dstTravelled = bicycleController.getDistanceTravelled();

	ostringstream os;
	os << std::setprecision(1) << std::fixed << avgVelocity  << " km/h" << endl << dstTravelled << " m";

	videoController.setSpeed(normalisedVelocity);
	videoController.getVideoTexture().draw(0, 0, ofGetWidth(), ofGetHeight());

	font.drawString(os.str(), 1000, 400);

	if (bShowDebug) {

		gui.begin();
		{
			bicycleController.drawGUI();
			videoController.drawGUI();

			ImGui::Spacing(); ImGui::Spacing();
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}
		gui.end();

	}

}

//--------------------------------------------------------------
void ofApp::exit() {
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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
		bicycleController.triggerSensor(BicycleController::SENSOR_KEYBOARD);
	}
	break;
	default:
		break;
	}

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

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
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
