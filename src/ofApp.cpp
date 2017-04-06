#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0);
	ofSetVerticalSync(false);
	ofSetFrameRate(1000);

	bShowDebug = true;

	bicycleController.setup();

	font.load(ofToDataPath("fonts/verdana.ttf"), 96, true);
}

//--------------------------------------------------------------
void ofApp::update(){
	
	bicycleController.update();

}

//--------------------------------------------------------------
void ofApp::draw(){

	float avgVelocity = bicycleController.getAverageVelocity();
	ostringstream os;
	os << std::setprecision(2) << avgVelocity << " km/h";

	font.drawString(os.str(), 1000, 400);

	if (bShowDebug) {

		gui.begin();
		{
			bicycleController.drawGUI();

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
		bShowDebug = !bShowDebug;
	}
	case ' ':
	{
		bicycleController.triggerSensor(BicycleController::SENSOR_KEYBOARD);
	}
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
