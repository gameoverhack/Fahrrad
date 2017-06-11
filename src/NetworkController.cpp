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

	bIsConnected = false;
	bNetworkNeedsUpdate = false;

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

	ipAddress[0] = 192;
	ipAddress[1] = 168;
	ipAddress[2] = 178;
	ipAddress[3] = 66;
	
	ipPort = 7000;

	nextNetworkMode = NETWORK_NONE;
	networkTimeout = 50.0f;

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
		// nothing
	}
	break;
	case NETWORK_SEND:
	{
		udp.Close();
		bIsConnected = false;
	}
	break;
	case NETWORK_RECV:
	{
		udp.Close();
		bIsConnected = false;
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
		connectSender();
	}
	break;
	case NETWORK_RECV:
	{
		connectReceiver();
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

			if (bIsConnected) {
				switch (currentNetworkMode) {
				case NETWORK_NONE:
				{
					// nothing
				}
				break;
				case NETWORK_SEND:
				{
					if (bNetworkNeedsUpdate) {
						//char c[1] = { '*' };
						//udp.Send(&c[0], 1);
						//ofSleepMillis(1);
						udp.Send(&riderSummary.chars[0], riderSummary.data[RS_DATA_SIZE] * sizeof(float));
						//ofSleepMillis(1);
						bNetworkNeedsUpdate = false;
					}
				}
				break;
				case NETWORK_RECV:
				{
					//if (!bNetworkNeedsUpdate) {
					//	char c[1] = { '0' };
					//	int recv = udp.Receive(&c[0], 1);
					//	if (recv == 1) {
					//		if (c[0] == '*') {
					//			bNetworkNeedsUpdate = true;
					//		}
					//	}
					//}
					//else {
						char c[1400] = { 0 };
						int recv = udp.Receive(&c[0], 1400);
						if (recv > 0) {
							if (riderSummary.data == nullptr) riderSummary.data = new float[(int)(recv / sizeof(float))];
							memcpy(&riderSummary.chars[0], &c[0], recv);
						}
						//bNetworkNeedsUpdate = false;
					//}

				}
				break;
				}
			}

			unlock();

		}

		ofSleepMillis(1);

	}

}

//--------------------------------------------------------------
void NetworkController::drawGUI() {

	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{

			ImGui::SliderInt("Network Timeout (millis)", &networkTimeout, 1, 1000);
			ImGui::Combo("Network Mode", (int*)&nextNetworkMode, networkModes);
			ImGui::InputInt4("IP Address", ipAddress);
			ImGui::InputInt("IP Port", &ipPort);
			 bool bReconnect = ImGui::Button("Reconnect");
			if (bReconnect) {
				lock();
				if (currentNetworkMode == NETWORK_SEND) connectSender();
				if (currentNetworkMode == NETWORK_RECV) connectReceiver();
				unlock();
			}
			ImGui::NewLine();
			lock();
			ImGui::Text("Network Mode: %s", networkModes[currentNetworkMode].c_str());
			unlock();

		}
		endGUI();
	}
}

//--------------------------------------------------------------
void NetworkController::setRiderSummary(const RiderSummaryUnion & rsu) {
	if (bUse) {
		lock();
		if (!bNetworkNeedsUpdate && ofGetElapsedTimeMillis() - lastNetworkTimeout >= networkTimeout && rsu.data != nullptr) {
			if (riderSummary.data == nullptr) riderSummary.data = new float[(int)rsu.data[RS_DATA_SIZE]];
			memcpy(&riderSummary.data[0], &rsu.data[0], rsu.data[RS_DATA_SIZE] * sizeof(float));
			bNetworkNeedsUpdate = true;
			lastNetworkTimeout = ofGetElapsedTimeMillis();
		}
		unlock();
	}
}

//--------------------------------------------------------------
const RiderSummaryUnion & NetworkController::getRiderSummary() {
	ofScopedLock lock(mutex);
	return riderSummary;
}

//--------------------------------------------------------------
bool NetworkController::connectSender() {
	ofLogNotice() << "Trying to connect sender";
	udp.Close();
	udp.Create();
	ostringstream os;
	for (int i = 0; i < 4; i++) {
		os << ipAddress[i];
		if (i != 3) os << ".";
	}
	bIsConnected = udp.Connect(os.str().c_str(), ipPort);
	udp.SetNonBlocking(true);
	return bIsConnected;
}

//--------------------------------------------------------------
bool NetworkController::connectReceiver() {
	ofLogNotice() << "Trying to connect receiver";
	udp.Close();
	udp.Create();
	bIsConnected = udp.Bind(ipPort);
	udp.SetNonBlocking(true);
	return bIsConnected;
}

//--------------------------------------------------------------
bool NetworkController::loadParameters() {
	return Serializer.loadClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool NetworkController::saveParameters() {
	return Serializer.saveClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
