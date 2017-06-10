#include "ViewController.h"

//--------------------------------------------------------------
ViewController::ViewController() {
	className = "ViewController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
ViewController::~ViewController() {

	ofLogNotice() << className << ": destructor";

	// kill the thread
	//waitForThread();

	this->IGuiBase::~IGuiBase(); // call base destructor
}

//--------------------------------------------------------------
void ViewController::setup() {

	ofLogNotice() << className << ": setup";

	// call base clase setup for now
	IGuiBase::setup();

	lastViewTimeout = ofGetElapsedTimeMillis() - viewTimeout;
	currentViewMode = VIEW_NONE;

	//startThread();
}

//--------------------------------------------------------------
void ViewController::setMode(ViewMode mode) {
	lock();
	nextViewMode = mode;
	unlock();
}

//--------------------------------------------------------------
void ViewController::setDefaults() {

	ofLogNotice() << className << ": setDefaults";

	viewTimeout = (int)(1000.0 / 60.0);
	nextViewMode = VIEW_NONE;

}

//--------------------------------------------------------------
void ViewController::update() {

	if (!bUse) return;

	// check thread safe GUI changes
	//lock();

	// check what mode we're in
	if (nextViewMode != currentViewMode) {
		changeMode();
	}

	if (bViewNeedsUpdate) {

		fbo.begin();
		{
			ofClear(0);

			switch (currentViewMode) {
			case VIEW_NONE:
			{
				// nothing
			}
			break;
			case VIEW_SEND:
			{
				renderSender();
			}
			break;
			case VIEW_RECV:
			{
				renderReciever();
			}
			break;
			}
		}
		fbo.end();

		bViewNeedsUpdate = false;
	}

	//unlock();

}

//--------------------------------------------------------------
void ViewController::renderSender() {

	backgroundFbo.draw(0, 0);

	if (topRiderInfo.size() > 0) {

		ostringstream os;

		const RiderInfo& riderInfo = topRiderInfo[topRiderInfo.size() - 2];
		int dayranking = riderInfo.dayranking + 1; // 0 ordered adjustment
		int allranking = riderInfo.allranking + 1; // 0 ordered adjustment
		float currentSpeed = riderInfo.currentSpeed;
		float currentKiloWatts = riderInfo.currentKiloWatts;
		float normalisedSpeed = riderInfo.normalisedSpeed;
		float distanceTravelled = riderInfo.distanceTravelled;
		string currentAnimal = "";//bicycleController->getAnimalFromIndex(riderInfo.currentAnimal);
		string currentDevice = "";//bicycleController->getDeviceFromIndex(riderInfo.currentDevice);


		os << std::setprecision(1) << std::fixed << currentSpeed << " km/h " << currentKiloWatts << " kW" << endl
			<< distanceTravelled << " m" << endl << dayranking << " // " << allranking << endl
			<< currentAnimal << endl << currentDevice;

		if (riderInfo.isActive) {
			ofDrawBitmapString(os.str(), 540.0f, 400.0f);
		}
	}
	
}

//--------------------------------------------------------------
void ViewController::renderReciever() {

	if (riderSummary.data != nullptr) {

		ostringstream os;

		float currentSpeed = riderSummary.data[RS_SPEED_CURRENT];
		float totalRiders = riderSummary.data[RS_RIDERS_TOTAL];
		float totalTime = riderSummary.data[RS_TIME_TOTAL];
		float totalDistance = riderSummary.data[RS_DISTANCE_TOTAL];

		os << std::setprecision(1) << std::fixed << currentSpeed << " km/h " << endl
			<< totalDistance << " m" << endl
			<< totalTime << " hours" << endl
			<< totalRiders << " riders" << endl;

		ofDrawBitmapString(os.str(), 960.0f, 400.0f);

	}

}

//--------------------------------------------------------------
void ViewController::changeMode() {

	// shutdown the current mode

	ofLogNotice() << "Shutting down: " << viewModes[currentViewMode];

	switch (currentViewMode) {
	case VIEW_NONE:
	{
		// nothing
	}
	break;
	case VIEW_SEND:
	{
		fbo.clear();
	}
	break;
	case VIEW_RECV:
	{
		fbo.clear();
	}
	break;
	}

	// setup for the next mode

	ofLogNotice() << "Setting up: " << viewModes[nextViewMode];

	switch (nextViewMode) {
	case VIEW_NONE:
	{
		// nothing
	}
	break;
	case VIEW_SEND:
	{
		fbo.allocate(1080.0f, 1920.0f, GL_RGB); // vertical layout
		renderSvgToFbo("images/Erg_Sender.svg", backgroundFbo, 1080.0f, 1920.0f);
	}
	break;
	case VIEW_RECV:
	{
		fbo.allocate(1920.0f, 1080.0f, GL_RGB); // horizontal layout
	}
	break;
	}

	// set current to next mode
	currentViewMode = nextViewMode;

}

//--------------------------------------------------------------
void ViewController::threadedFunction() {

	while (isThreadRunning()) {

		if (bUse) {

			//lock();

			//unlock();

		}

		ofSleepMillis(1);

	}

}

//--------------------------------------------------------------
void ViewController::drawGUI() {

	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{
			ImGui::SliderInt("View Timeout (millis)", &viewTimeout, 1, 1000);
			ImGui::Combo("View Mode", (int*)&nextViewMode, viewModes);
			ImGui::NewLine();
			//lock();
			ImGui::Text("View Mode: %s", viewModes[currentViewMode].c_str());
			//unlock();

		}
		endGUI();
	}
}

//--------------------------------------------------------------
void ViewController::setData(const RiderSummaryUnion & rsu, const vector<RiderInfo>& tri) {
	if (bUse) {
		//lock();
		if (!bViewNeedsUpdate && ofGetElapsedTimeMillis() - lastViewTimeout >= viewTimeout && rsu.data != nullptr) {
			if (riderSummary.data == nullptr) riderSummary.data = new float[(int)rsu.data[0]];
			memcpy(&riderSummary.data[0], &rsu.data[0], rsu.data[0]);
			topRiderInfo = tri;
			bViewNeedsUpdate = true;
			lastViewTimeout = ofGetElapsedTimeMillis();
		}
		//unlock();
	}
}

//--------------------------------------------------------------
const ofFbo & ViewController::getFBO() {
	//ofScopedLock lock(mutex);
	return fbo;
}

//--------------------------------------------------------------
void ViewController::renderSvgToFbo(const string & filePath, ofFbo & svgFbo, float w, float h) {
	ofxSVG svg;
	svg.load(filePath);
	svgFbo.allocate(w, h, GL_RGB, 8);
	svgFbo.begin();
	{
		ofClear(255);
		ofSetColor(255);
		svg.draw();
	}
	svgFbo.end();
}

//--------------------------------------------------------------
bool ViewController::loadParameters() {
	return Serializer.loadClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool ViewController::saveParameters() {
	return Serializer.saveClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
