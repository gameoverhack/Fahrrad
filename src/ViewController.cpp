#include "ViewController.h"

//--------------------------------------------------------------
ViewController::ViewController() {
	className = "ViewController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
ViewController::~ViewController() {

	ofLogNotice() << className << ": destructor";

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
			m.typeDE = XML.getValue("milestone:nameDE", "", i);
			m.typeEN = XML.getValue("milestone:nameEN", "", i);
			ofLogNotice() << "Milestone: " << i << " = (" << m.value << ", " << m.typeDE << ", " << m.typeEN << ")";
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
			m.typeDE = XML.getValue("milestone:nameDE", "", i);
			m.typeEN = XML.getValue("milestone:nameEN", "", i);
			ofLogNotice() << "Milestone: " << i << " = (" << m.value << ", " << m.typeDE << ", " << m.typeEN << ")";
			milestonesWatts.push_back(m);
		}
	}
	else {
		ofLogError() << "XML milestonesWatt could not be loaded";
	}

	timeoutSinceLastRider = 4000;
	timeSinceLastRider = ofGetElapsedTimeMillis() - timeoutSinceLastRider;

}

//--------------------------------------------------------------
void ViewController::setMode(ViewMode mode) {
	nextViewMode = mode;
}

//--------------------------------------------------------------
void ViewController::setDefaults() {

	ofLogNotice() << className << ": setDefaults";

	viewTimeout = 150;
	nextViewMode = VIEW_NONE;
	bShowPulseSignal = false;

}

//--------------------------------------------------------------
void ViewController::update() {

	if (!bUse) return;

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

}

//--------------------------------------------------------------
void ViewController::renderSender() {
	//ofEnableBlendMode(OF_BLENDMODE_ALPHA );

	if (topRiderInfo.size() > 0 && riderSummary.data != nullptr) {

		// get data from current rider info to display

		const RiderInfo& riderInfo = topRiderInfo[topRiderInfo.size() - 2];
		
		string watts = string(riderInfo.isActive ? getString(riderInfo.currentKiloWatts) : getString(lastTopWatts));
		string speedCurrent = getString(riderInfo.currentSpeed);
		string speedHigh = string(riderInfo.isActive ? getString(riderInfo.topSpeed) : getString(lastTopSpeed));
		string distanceToday = getString(riderSummary.data[RS_DISTANCE_DAY] / 1000.0f, 1);
		string distanceRider = getString(riderInfo.distanceTravelled / 1000.0f, 2);
		int total = riderInfo.time / 1000.0; int minutes = (total / 60) % 60; int seconds = total % 60;
		string timeRider = getString(minutes, 0, 2) + ":" + getString(seconds, 0, 2);
		string heartRate = getString(171); // TODO!!!
		
		// calculate current watts-to-device TODO: make english/german work with artikel

		string deviceDE = "";
		string deviceEN = "";

		for (int i = 0; i < milestonesWatts.size(); i++) {
			if ((riderInfo.isActive ? riderInfo.currentKiloWatts : lastTopWatts) >= milestonesWatts[i].value) {
				deviceDE = milestonesWatts[i].typeDE;
				deviceEN = milestonesWatts[i].typeEN;
			}
		}
		
		// calculate and draw speedo readout - TODO: parameterise hard coded settings if necessary

		float normalAngle = 218.5f;
		float maxDisplaySpeed = 70.0f;

		ofPushMatrix();

		// draw white background
		ofSetColor(255);
		ofDrawRectangle(0.0f, 0.0f, 1080.0f, 1920.0f);

		ofTranslate(78.029, 243.467);

		dialGreyFbo.draw(0, 0); // background grey part of speedo

		// draw the red curve part of the speedo (ie., current speed)

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

		// draw the red line part of the speedo (ie., high speed) - TODO: check if it should be on top, underneath etc, color?

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

		dialMaskFbo.draw(0, 0); // draw the mask

		ofPopMatrix();

		backgroundFbo.draw(0, 0); // draw all the rest of the static info

		ofSetColor(0);

		// draw all the dynamic text

		// draw current device for watts
		// need this hack because ofxTextAlign can't handle unicode strings :(

		//fDeviceBold.draw(deviceDE, 45.578, 71.999, ofxTextAlign::HORIZONTAL_ALIGN_LEFT | ofxTextAlign::VERTICAL_ALIGN_TOP);
		fDeviceBold.drawString(deviceDE, 45.578, 71.999 + fDeviceBold.getLineHeight() + fDeviceBold.getDescenderHeight());
		ofSetColor(112, 111, 111);
		//fDeviceItalic.draw(deviceEN, 50.125, 169.875, ofxTextAlign::HORIZONTAL_ALIGN_LEFT | ofxTextAlign::VERTICAL_ALIGN_TOP);
		fDeviceItalic.drawString(deviceEN, 50.125, 169.875 + fDeviceItalic.getLineHeight() + fDeviceItalic.getDescenderHeight());

		// draw watts
		ofSetColor(0);
		fWattsCurrent.draw(watts, 871.709, 142.148, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);

		// draw speed - current and max
		fSpeedCurrent.draw(speedCurrent, 537.281, 758.004, ofxTextAlign::HORIZONTAL_ALIGN_CENTER | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
		ofSetColor(130, 198, 116); // TODO: check this color!
		fSpeedHigh.draw(speedHigh, 913.814, 307.874, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);

		// draw info about distance and time
		ofSetColor(0);
		fDistanceTime.draw(distanceToday, 206.733, 1051.155, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
		fDistanceTime.draw(distanceRider, 552.785, 1051.155, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
		fDistanceTime.draw(timeRider, 900.819, 1051.155, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);

		// draw heartrate graph
		ofFill();
		ofSetColor(238, 82, 83);

		// NB: using the last entry to hack encode the last index of pulse recorded
		int lastBPMIndex = pulseData[35].bpm; 

		// sort the bpm from latest to oldest
		vector<float> bpm(35);
		bpm[34] = pulseData[lastBPMIndex].bpm;

		int bpmIndex = 0;
		if (lastBPMIndex + 1 < 35) {
			for (int i = lastBPMIndex + 1; i < 35; i++) {
				bpm[bpmIndex] = pulseData[i].bpm;
				bpmIndex++;
			}
		}

		if (lastBPMIndex - 1 > 0) {
			for (int i = 0; i <= lastBPMIndex - 1; i++) {
				bpm[bpmIndex] = pulseData[i].bpm;
				bpmIndex++;
			}
		}

		// show it onscreen
		for (int i = 0; i < 35; i++) {
			float x = 83.726f + i * 21.468f;
			float y = 1436.69f;
			float w = 10.716;
			float h = (CLAMP(bpm[i], 60, 200) - 60) * (145.068f / (200.0f - 60.0f));
			ofDrawRectangle(x, y, w, -h);
		}
		ofNoFill();

		// draw pulse signal
		if (bShowPulseSignal) {
			ofSetColor(0, 73, 148);
			ofMesh mesh;
			mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
			for (int i = 0; i < rawPulseData.size(); i++) {
				mesh.addVertex(ofPoint(77.533f + i, 1436.69f - CLAMP(rawPulseData[i].signal, 0, 1024) * (145.068f / 1024.0f), 0));
			}
			mesh.draw();
		}
		
		// draw heart rate in the heart!
		ofSetColor(0); 
		fHeartRate.draw(ofToString(CLAMP(pulseData[lastBPMIndex].bpm, 0, 200)), 937.0f, 1373.888, ofxTextAlign::HORIZONTAL_ALIGN_CENTER | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);

		// draw high scores
		for (int i = 0; i < topRiderInfo.size(); i++) {
			
			float offsetY = i * 53.5f;
			
			// get data for each rider high score
			string rank = getString(topRiderInfo[i].allranking + 1) + ".";
			string topspeed = getString(topRiderInfo[i].topSpeed, 1) + " km/h";
			string distance = getString(topRiderInfo[i].distanceTravelled / 1000.0f, 2) +" km";
			string date = getString(topRiderInfo[i].day, 0, 2) + "." + getString(topRiderInfo[i].month, 0, 2) + "." + getString(topRiderInfo[i].year);
			
			if (date == "00.00.0") continue; // ie., no rider? don't show current info in highscores

			if (i == topRiderInfo.size() - 2) {
				ofSetColor(255); // current
			}
			else {
				ofSetColor(50); // 1,2,3,last
			}
			
			// draw rank, top speed, distance and date
			fHighScores.draw(rank, 154.06, offsetY + 1669.842, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
			fHighScores.draw(topspeed, 446.652, offsetY + 1669.842, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
			fHighScores.draw(distance, 701.179, offsetY + 1669.842, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
			fHighScores.draw(date, 1009.421, offsetY + 1669.842, ofxTextAlign::HORIZONTAL_ALIGN_RIGHT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);

		}

		// hide watts and top speed if no rider
		if (riderInfo.isActive) {
			timeSinceLastRider = ofGetElapsedTimeMillis();
			lastTopSpeed = MAX(lastTopSpeed, riderInfo.currentSpeed);
			lastTopWatts = MAX(lastTopWatts, riderInfo.currentKiloWatts);
		}
		if (ofGetElapsedTimeMillis() - timeSinceLastRider > timeoutSinceLastRider) {
			ofFill();
			ofSetColor(255);
			ofDrawRectangle(600, 30, 470, 160);
			ofDrawRectangle(850, 240, 215, 75);
			lastTopSpeed = lastTopWatts = 0.0f;
		}
	}
	else {
		backgroundFbo.draw(0, 0); // for now draw this without white background to show we are loading
	}

	
}

//--------------------------------------------------------------
void ViewController::renderReciever() {

	if (riderSummary.data != nullptr) {

		// get and format data from riderSummary
		float tP = riderSummary.data[RS_DISTANCE_TOTAL] / 1000.0f / 4664.0f * 100.0f; int tPprecision = (tP < 100 ? (tP < 10 ? 2 : 1) : 0);
		string percentage = getString(tP, tPprecision) + " %";
		string speedCurrent = getString(riderSummary.data[RS_SPEED_CURRENT]);
		string riderTotal = getString(riderSummary.data[RS_RIDERS_TOTAL] + riderSummary.data[RS_IS_ACTIVE]); // count the active rider!
		float total = riderSummary.data[RS_TIME_TOTAL]; int seconds = int(total) % 60; int minutes = (int(total / 60) % 60); int hours = int(total / 60) / 60.0f; // format hhh:mm
		string timeTotal = getString(hours, 0, 2) + ":" + getString(minutes, 0, 2) + ":" + getString(seconds, 0, 2);
		float dDT = riderSummary.data[RS_DISTANCE_DAY] / 1000.0f; int dDTprecision = (dDT < 1000 ? (dDT < 100 ? (dDT < 10 ? 3 : 2) : 1) : 0); // display floating point when less than 100
		string distanceToday = getString(dDT, dDTprecision);
		float dTT = riderSummary.data[RS_DISTANCE_TOTAL] / 1000.0f; int dTTprecision = (dTT < 1000 ? (dTT < 100 ? (dTT < 10 ? 3 : 2) : 1) : 0); // display floating point when less than 1000
		string distanceTotal = getString(dTT, dTTprecision);
		string distanceGermany = "4664";

		// calculate current speed equivalent for animals

		string animalDE = "";
		string animalEN = "";

		//int currentAnimal = -1;
		for (int i = 0; i < milestonesSpeed.size(); i++) {
			if (riderSummary.data[RS_SPEED_CURRENT] >= milestonesSpeed[i].value) {
				animalDE = milestonesSpeed[i].typeDE;
				animalEN = milestonesSpeed[i].typeEN;
			}
		}

		// draw white background
		ofSetColor(255);
		ofDrawRectangle(0.0f, 0.0f, 1080.0f, 1920.0f);

		backgroundFbo.draw(0, 0); // draw all the rest of the static info

		// draw info text
		ofSetColor(255);
		fPercentageDone.draw(percentage, 487.359, 287.827, ofxTextAlign::HORIZONTAL_ALIGN_CENTER | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
		ofSetColor(0, 73, 148);
		fSpeedCurrent2.draw(speedCurrent, 46.787, 421.78, ofxTextAlign::HORIZONTAL_ALIGN_LEFT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);

		fAnimalBold.drawString(animalDE, 46.787, 490.353 + fAnimalBold.getDescenderHeight());
		ofSetColor(112, 111, 111);
		fAnimalItalic.drawString(animalEN, 46.787, 563.939 + fAnimalItalic.getDescenderHeight());

		ofSetColor(0, 73, 148);
		fDistanceTime2.draw(riderTotal, 46.787, 960.345, ofxTextAlign::HORIZONTAL_ALIGN_LEFT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
		fDistanceTime2.draw(timeTotal, 208.414, 960.345, ofxTextAlign::HORIZONTAL_ALIGN_LEFT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
		fDistanceTime2.draw(distanceToday, 443.772, 960.345, ofxTextAlign::HORIZONTAL_ALIGN_LEFT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
		fDistanceTime2.draw(distanceTotal, 646.457, 960.345, ofxTextAlign::HORIZONTAL_ALIGN_LEFT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);
		fDistanceTime2.draw(distanceGermany, 867.313, 960.345, ofxTextAlign::HORIZONTAL_ALIGN_LEFT | ofxTextAlign::VERTICAL_ALIGN_BOTTOM);

		// draw daily distance graph

		ofFill();

		int nDays = (int)riderSummary.data[RS_NUM_DAYS];
		for (int i = 0; i < MIN(nDays, 171); i++) {

			float x = 49.322 + i * 5.786f;
			float y = 797.268f;
			float w = 2.888f;
			float dist = riderSummary.data[RS_DATA_START + i] / 1000.0f;
			float h = dist * (85.095f / 40.0f);
			h = CLAMP(h, 0, 50.0f * (85.095f / 40.0f));
			//if (i == 170) ofSetColor(255, 0, 0);
			ofDrawRectangle(x, y, w, -h);

		}


		// calculate outline of germany
		//float mx = ofGetMouseX();
		//CLAMP(mx, 1.0f, ofGetWidth());
		//float step = mx / ofGetWidth() * 2.0 * 100.0f;
		
		float width = 6.0f;
		ofPoint finalPosition;

		if (tP <= 100) {
			ofMesh mesh;
			setDistanceMesh(mesh, finalPosition, width, tP);
			ofFill();
			ofSetColor(255, 79, 56, 255);
			mesh.draw();
		}
		else {
			ofMesh mesh1;
			setDistanceMesh(mesh1, finalPosition, width, 100);
			ofFill();
			ofSetColor(255, 79, 56, 255);
			mesh1.draw();
			ofMesh mesh2;
			float remainder = (int)tP % 100;
			setDistanceMesh(mesh2, finalPosition, width, remainder);
			ofFill();
			ofSetColor(130, 198, 116, 255);
			mesh2.draw();
		}

		// draw current position around germany
		ofSetColor(0, 73, 148);
		ofDrawCircle(finalPosition, width);
		//ofDrawBitmapString(ofToString(num), vL[2]);

	}
	else {
		backgroundFbo.draw(0, 0);
	}

}
//--------------------------------------------------------------
void ViewController::setDistanceMesh(ofMesh & mesh, ofPoint & finalPosition, float width, float pct) {
	
	mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);

	//float W = 6.0; // width of line - TODO: parameter
	ofPoint vL[4];

	int num = (pct / 100.0f * polyDEOutlines.size()) + 11; // step // 11 is Dresden startpoint

	// from https://stackoverflow.com/questions/14514543/opengl-es-2-0-dynamically-change-line-width
	for (int j = 10; j < num; j++) {

		int index1 = j - 1;
		int index2 = j - 0;
		if (index1 >= polyDEOutlines.size()) index1 = index1 - polyDEOutlines.size() + 10;
		if (index2 >= polyDEOutlines.size()) index2 = index2 - polyDEOutlines.size() + 10;
		ofPoint p1 = polyDEOutlines[index1];
		ofPoint p2 = polyDEOutlines[index2];
		ofPoint  v = p2 - p1;

		v /= v.length();  // make it a unit vector

		ofPoint vp(-v.y, v.x);  // compute the vector perpendicular to v

		vL[0] = p1 + width / 2 * vp;
		vL[1] = p1 - width / 2 * vp;
		vL[2] = p2 + width / 2 * vp;
		vL[3] = p2 - width / 2 * vp;

		mesh.addVertex(vL[0]);
		mesh.addVertex(vL[1]);
		mesh.addVertex(vL[2]);
		mesh.addVertex(vL[3]);

	}

	finalPosition = vL[2];

}

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
		renderToFbo("Erg_Sender", backgroundFbo, 1080.0f, 1920.0f, ofColor(255, 255, 255, 0));
		renderToFbo("Erg_DialRed", dialRedFbo, 923.422f, 923.422f, ofColor(0, 0, 0, 0));
		renderToFbo("Erg_DialMax", dialMaxFbo, 923.422f, 923.422f, ofColor(0, 0, 0, 0));
		renderToFbo("Erg_DialGrey", dialGreyFbo, 923.422f, 923.422f, ofColor(0, 0, 0, 0));
		renderToFbo("Erg_DialMask", dialMaskFbo, 923.422f, 923.422f, ofColor(0, 0, 0, 0));
		
		fDeviceBold.load("fonts/NotBd.ttf",38);
		fDeviceItalic.load("fonts/NotRgI.ttf", 17);

		fWattsCurrent.load("fonts/OpenSans-Bold.ttf", 56);
		fSpeedCurrent.load("fonts/OpenSans-Bold.ttf", 166);
		fSpeedHigh.load("fonts/OpenSans-Regular.ttf", 42);
		fDistanceTime.load("fonts/OpenSans-Bold.ttf", 45);
		fHeartRate.load("fonts/OpenSans-Bold.ttf", 30);
		fHighScores.load("fonts/Roboto-Regular.ttf", 26);

	}
	break;
	case VIEW_RECV:
	{

		fbo.allocate(1080.0f, 1080.0f, GL_RGBA); // horizontal/square layout
		renderToFbo("Erg_Receiver", backgroundFbo, 1080.0f, 1080.0f, ofColor(255, 255, 255, 0));
		
		fAnimalBold.load("fonts/NotMd_.ttf", 24);
		fAnimalItalic.load("fonts/NotRgI.ttf", 24);

		fPercentageDone.load("fonts/OpenSans-Bold.ttf", 42);
		fSpeedCurrent2.load("fonts/OpenSans-Bold.ttf", 65);
		fDistanceTime2.load("fonts/OpenSans-Bold.ttf", 36);

		ofxSVG svgDEOutline;
		svgDEOutline.load("images/Erg_DEOutline.svg");
		ofPath p = svgDEOutline.getPathAt(0);
		p.setPolyWindingMode(OF_POLY_WINDING_ODD);
		vector<ofPolyline>& lines = const_cast<vector<ofPolyline>&>(p.getOutline());
		polyDEOutlines = lines[0].getResampledByCount(800);

	}
	break;
	}

	// set current to next mode
	currentViewMode = nextViewMode;

}

//--------------------------------------------------------------
void ViewController::drawGUI() {

	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{
			ImGui::SliderInt("View Timeout (millis)", &viewTimeout, 1, 1000);
			ImGui::Combo("View Mode", (int*)&nextViewMode, viewModes);
			ImGui::Checkbox("Show Pulse Signal", &bShowPulseSignal);
			ImGui::NewLine();
			ImGui::Text("View Mode: %s", viewModes[currentViewMode].c_str());
			
		}
		endGUI();
	}
}

//--------------------------------------------------------------
void ViewController::setData(const RiderSummaryUnion & rsu, const vector<RiderInfo>& tri, const vector<PulseData>& pd, const vector<PulseData>& rd) {
	if (bUse) {
		if (!bViewNeedsUpdate && ofGetElapsedTimeMillis() - lastViewTimeout >= viewTimeout && rsu.data != nullptr) {
			if (riderSummary.data == nullptr) riderSummary.data = new float[(int)rsu.data[0]];
			memcpy(&riderSummary.data[0], &rsu.data[0], rsu.data[0] * sizeof(float));
			topRiderInfo = tri;
			pulseData = pd;
			rawPulseData = rd;
			bViewNeedsUpdate = true;
			lastViewTimeout = ofGetElapsedTimeMillis();
		}
	}
}

//--------------------------------------------------------------
const ofFbo & ViewController::getFBO() {
	return fbo;
}

//--------------------------------------------------------------
void ViewController::renderToFbo(string fileName, ofFbo & imageFbo, float w, float h, ofColor c) {
	string filePath = "images/" + fileName + ".png";

	//ofxSVG svg;
	//svg.load(filePath);
	
	ofImage img;
	img.load(filePath);

	imageFbo.allocate(w, h, GL_RGBA); //, 8 multisampling not working on linux :(
	imageFbo.begin();
	{
		ofClear(c);
		ofSetColor(255, 255, 255, 255);
		img.draw(-1, -1, imageFbo.getWidth() + 1, imageFbo.getHeight() + 1);
	}
	imageFbo.end();
}

//--------------------------------------------------------------
bool ViewController::loadParameters() {
	cout << "load: " << configPath << endl;
	return Serializer.loadClass(ofToDataPath("configs/" + configPath + "/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool ViewController::saveParameters() {
	cout << "save: " << configPath << endl;
	return Serializer.saveClass(ofToDataPath("configs/" + configPath + "/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
