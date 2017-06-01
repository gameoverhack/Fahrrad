#include "NetworkController.h"

//--------------------------------------------------------------
NetworkController::NetworkController() {
	className = "NetworkController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
NetworkController::~NetworkController() {

	ofLogNotice() << className << ": destructor";

	// kill the thread
	waitForThread();

	this->IGuiBase::~IGuiBase(); // call base destructor
}

//--------------------------------------------------------------
void NetworkController::setup() {
	ofLogNotice() << className << ": setup";

	// call base clase setup for now
	IGuiBase::setup();

	currentNetworkMode = NETWORK_NONE; // so we force sensor mode change in update
	lastNetworkTimeout = ofGetElapsedTimeMillis() - networkTimeout;

	startThread();
}

//--------------------------------------------------------------
void NetworkController::setMode(NetworkMode mode) {
	lock();
	nextNetworkMode = mode;
	unlock();
}

//--------------------------------------------------------------
void NetworkController::setDefaults() {
	ofLogNotice() << className << ": setDefaults";

	nextNetworkMode = NETWORK_NONE;
	networkTimeout = 2000;

}

//--------------------------------------------------------------
void NetworkController::update() {

	if (!bUse) return;

	// check thread safe GUI changes
	lock();

	// check what mode we're in
	if (nextNetworkMode != currentNetworkMode) {
		changeMode();
	}

	unlock();

}

//--------------------------------------------------------------
void NetworkController::changeMode() {

	// shutdown the current mode

	ofLogNotice() << "Shutting down: " << networkModes[currentNetworkMode];

	switch (currentNetworkMode) {
	case NETWORK_NONE:
	{

	}
	break;
	case NETWORK_SEND:
	{

	}
	break;
	case NETWORK_RECV:
	{

	}
	break;
	}

	// setup for the next mode

	ofLogNotice() << "Setting up: " << networkModes[nextNetworkMode];

	switch (nextNetworkMode) {
	case NETWORK_NONE:
	{

	}
	break;
	case NETWORK_SEND:
	{

	}
	break;
	case NETWORK_RECV:
	{

	}
	break;
	}

	// set current to next mode
	currentNetworkMode = nextNetworkMode;

}

//--------------------------------------------------------------
void NetworkController::threadedFunction() {

	while (isThreadRunning()) {

		if (bUse) {

			lock();

			// grab the current mode
			int thisCurrentMode = currentNetworkMode;

			unlock();


			switch (thisCurrentMode) {
			case NETWORK_NONE:
			{

			}
			break;
			case NETWORK_SEND:
			{

			}
			break;
			case NETWORK_RECV:
			{

			}
			break;
			}

		}

		ofSleepMillis(1);

	}

}

//--------------------------------------------------------------
void NetworkController::drawGUI() {

	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{

			ImGui::SliderInt("Network Timeout (millis)", &networkTimeout, 1000, 20000);
			ImGui::NewLine();
			lock();
			ImGui::Text("Network Mode: %s", networkModes[currentNetworkMode].c_str());
			unlock();

		}
		endGUI();
	}
}

//--------------------------------------------------------------
bool NetworkController::loadParameters() {
	return Serializer.loadClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool NetworkController::saveParameters() {
	return Serializer.saveClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
