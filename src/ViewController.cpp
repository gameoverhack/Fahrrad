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

	// load milestones
	ofxXmlSettings XML;
	if (XML.loadFile("xml/milestonesSpeed.xml")) {
		ofLogNotice() << "XML milestonesSpeed loaded";
		XML.pushTag("milestonesSpeed", 0);
		int numMilestones = XML.getNumTags("milestone");
		for (int i = 0; i < numMilestones; i++) {
			MileStone m;
			m.value = XML.getValue("milestone:speed", 0., i);
			m.type = XML.getValue("milestone:name", "", i);
			ofLogNotice() << "Milestone: " << i << " = (" << m.value << ", " << m.type << ")";
			milestonesSpeed.push_back(m);
		}
	}
	else {
		ofLogError() << "XML milestonesSpeed could not be loaded";
	}

	if (XML.loadFile("xml/milestonesWatts.xml")) {
		ofLogNotice() << "XML milestonesWatt loaded";
		XML.pushTag("milestonesWatt", 0);
		int numMilestones = XML.getNumTags("milestone");
		for (int i = 0; i < numMilestones; i++) {
			MileStone m;
			m.value = XML.getValue("milestone:watt", 0., i);
			m.type = XML.getValue("milestone:name", "", i);
			ofLogNotice() << "Milestone: " << i << " = (" << m.value << ", " << m.type << ")";
			milestonesWatts.push_back(m);
		}
	}
	else {
		ofLogError() << "XML milestonesWatt could not be loaded";
	}

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
	//ofEnableBlendMode(OF_BLENDMODE_ALPHA );

	if (topRiderInfo.size() > 0 && riderSummary.data != nullptr) {

		const RiderInfo& riderInfo = topRiderInfo[topRiderInfo.size() - 2];
		
		string watts = getString(riderInfo.currentKiloWatts);
		string speedCurrent = getString(riderInfo.currentSpeed);
		string speedHigh = getString(riderInfo.topSpeed);
		string distanceToday = getString(riderSummary.data[RS_DISTANCE_DAY] / 1000.0f, 1);
		string distanceRider = getString(riderInfo.distanceTravelled / 1000.0f, 2);
		int total = riderInfo.time / 1000.0; int minutes = (total / 60) % 60; int seconds = total % 60;
		string timeRider = getString(minutes, 0, 2) + ":" + getString(seconds, 0, 2);
		string heartRate = getString(171); // TODO!!!
		

		//int currentAnimal = -1;
		//for (int i = 0; i < milestonesSpeed.size(); i++) {
		//	if (riderInfo.currentSpeed < milestonesSpeed[i].value) {
		//		if (i > 0) {
		//			currentAnimal = i;//milestonesSpeed[i].type;
		//		}
		//		break;
		//	}
		//}

		string deviceDE = "";
		string deviceEN = "";

		for (int i = 0; i < milestonesWatts.size(); i++) {
			if (riderInfo.currentKiloWatts < milestonesWatts[i].value) {
				if (i > 0) {
					//currentRider.currentDevice = i;
					deviceDE = deviceEN = milestonesWatts[i].type;
				}
				break;
			}
		}
		
		float normalAngle = 218.5f;
		float maxDisplaySpeed = 70.0f;

		ofPushMatrix();

		ofSetColor(255);
		ofDrawRectangle(0.0f, 0.0f, 1080.0f, 1920.0f);

		ofTranslate(78.029, 243.467);

		dialGreyFbo.draw(0, 0);

		ofPushMatrix();
		{
			ofTranslate(dialRedFbo.getWidth() / 2.0f, dialRedFbo.getHeight() / 2.0f);
			ofRotateZ(normalAngle * MIN(riderInfo.currentSpeed, maxDisplaySpeed) / maxDisplaySpeed - normalAngle);
			ofTranslate(-dialRedFbo.getWidth() / 2.0f, -dialRedFbo.getHeight() / 2.0f);
			if (riderInfo.currentSpeed < maxDisplaySpeed / 2.0) {
				dialRedFbo.getTexture().drawSubsection(dialRedFbo.getWidth() / 2.0, 0, dialRedFbo.getWidth() / 2.0, dialRedFbo.getHeight(), dialRedFbo.getWidth() / 2.0, 0);
			}
			else {
				dialRedFbo.draw(0, 0);
			}
		}
		ofPopMatrix();

		ofPushMatrix();
		{
			ofTranslate(dialMaxFbo.getWidth() / 2.0f, dialMaxFbo.getHeight() / 2.0f);
			ofRotateZ(normalAngle * MIN(riderInfo.topSpeed, maxDisplaySpeed) / maxDisplaySpeed - normalAngle);
			ofTranslate(-dialMaxFbo.getWidth() / 2.0f, -dialMaxFbo.getHeight() / 2.0f);
			if (riderInfo.topSpeed < maxDisplaySpeed / 2.0) {
				dialMaxFbo.getTexture().drawSubsection(dialMaxFbo.getWidth() / 2.0, 0, dialMaxFbo.getWidth() / 2.0, dialMaxFbo.getHeight(), dialMaxFbo.getWidth() / 2.0, 0);
			}
			else {
				dialMaxFbo.draw(0, 0);
			}
		}
		ofPopMatrix();

		dialMaskFbo.draw(0, 0);

		ofPopMatrix();

		backgroundFbo.draw(0, 0);

		ofSetColor(0);

		// need this hack because ofxTextAlign can't handle unicode strings :(
		//fDeviceBold.draw(deviceDE, 45.578, 71.999, ofxTextAlign::HORIZONTAL_ALIGN_LEFT | ofxTextAlign::VERTICAL_ALIGN_TOP);
		fDeviceBold.drawString(deviceDE, 45.578, 71.999 + fDeviceBold.getLineHeight() + fDeviceBold.getDescenderHeight());
		ofSetColor(112, 111, 111);
		//fDeviceItalic.draw(deviceEN, 50.125, 169.875, ofxTextAlign::HORIZONTAL_ALIGN_LEFT | ofxTextAlign::VERTICAL_ALIGN_TOP);
		fDeviceItalic.drawString(deviceEN, 50.125, 169.875 + fDeviceItalic.getLineHeight() + fDeviceItalic.getDescenderHeight());

		ofSetColor(0);
		fWattsCurrent.draw(watts, 871.709, 142.148, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
		fSpeedCurrent.draw(speedCurrent, 537.281, 818.94, ofxTextAlign::HORIZONTAL_ALIGN_CENTER | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
		ofSetColor(238, 82, 83);
		fSpeedHigh.draw(speedHigh, 913.814, 307.874, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
		ofSetColor(0);
		fDistanceTime.draw(distanceToday, 206.733, 1051.155, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
		fDistanceTime.draw(distanceRider, 552.785, 1051.155, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
		fDistanceTime.draw(timeRider, 900.819, 1051.155, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);

		for (int i = 0; i < topRiderInfo.size(); i++) {
			
			float offsetY = i * 53.5f;
			
			string rank = getString(topRiderInfo[i].allranking + 1) + ".";
			string topspeed = getString(topRiderInfo[i].topSpeed, 1) + " km/h";
			string distance = getString(topRiderInfo[i].distanceTravelled / 1000.0f, 2) +" km";
			string date = getString(topRiderInfo[i].day, 0, 2) + "." + getString(topRiderInfo[i].month, 0, 2) + "." + getString(topRiderInfo[i].year);
			
			if (date == "00.00.0") continue; // ie., no rider?

			if (i == topRiderInfo.size() - 2) {
				ofSetColor(255);
			}
			else {
				ofSetColor(50);
			}
			
			fHighScores.draw(rank, 154.06, offsetY + 1669.842, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
			fHighScores.draw(topspeed, 446.652, offsetY + 1669.842, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
			fHighScores.draw(distance, 701.179, offsetY + 1669.842, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
			fHighScores.draw(date, 1009.421, offsetY + 1669.842, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);

		}
		

	}
	else {
		backgroundFbo.draw(0, 0);
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
		fbo.allocate(1080.0f, 1920.0f, GL_RGBA); // vertical layout
		renderSvgToFbo("images/Erg_Sender.svg", backgroundFbo, 1080.0f, 1920.0f, ofColor(255, 255, 255, 0));
		renderSvgToFbo("images/Erg_DialRed.svg", dialRedFbo, 923.422f, 923.422f, ofColor(0, 0, 0, 0));
		renderSvgToFbo("images/Erg_DialMax.svg", dialMaxFbo, 923.422f, 923.422f, ofColor(0, 0, 0, 0));
		renderSvgToFbo("images/Erg_DialGrey.svg", dialGreyFbo, 923.422f, 923.422f, ofColor(0, 0, 0, 0));
		renderSvgToFbo("images/Erg_DialMask.svg", dialMaskFbo, 923.422f, 923.422f, ofColor(0, 0, 0, 0));
		
		fDeviceBold.load("fonts/NotBd.ttf",44);
		fDeviceItalic.load("fonts/NotRgI.ttf", 17);

		fWattsCurrent.load("fonts/OpenSans-Bold.ttf", 56);
		fSpeedCurrent.load("fonts/OpenSans-Bold.ttf", 260);
		fSpeedHigh.load("fonts/OpenSans-Regular.ttf", 42);
		fDistanceTime.load("fonts/OpenSans-Bold.ttf", 45);
		fHeartRate.load("fonts/OpenSans-Bold.ttf", 44);
		fHighScores.load("fonts/Roboto-Regular.ttf", 26);

	}
	break;
	case VIEW_RECV:
	{
		fbo.allocate(1920.0f, 1080.0f, GL_RGBA); // horizontal layout
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
void ViewController::renderSvgToFbo(string filePath, ofFbo & svgFbo, float w, float h, ofColor c) {
	ofxSVG svg;
	svg.load(filePath);
	svgFbo.allocate(w, h, GL_RGBA, 8);
	svgFbo.begin();
	{
		ofClear(c);
		ofSetColor(255, 255, 255, 255);
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
