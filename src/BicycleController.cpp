#include "BicycleController.h"

bool RiderRankFunction(const RiderInfo& a, const RiderInfo& b) { return (a.topSpeed > b.topSpeed); }

//--------------------------------------------------------------
BicycleController::BicycleController() {
	className = "BicycleController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
BicycleController::~BicycleController() {
	ofLogNotice() << className << ": destructor";

	// kill the thread
	waitForThread();

	this->IGuiBase::~IGuiBase(); // call base destructor
}

//--------------------------------------------------------------
void BicycleController::setup() {
	ofLogNotice() << className << ": setup";

	// call base clase setup for now
	IGuiBase::setup();

	currentSensorMode = SENSOR_NONE; // so we force sensor mode change in update
	currentAverageVelocity = 0;
	lastMeasuredVelocity = 0;
	timeSinceLastSensor = 0;
	lastVelocityTimeout = 0;
	simulateVelocity = 0;
	distanceTravelled = 0; // neters

	bIsRiderActive = false;

	if (bRecordRiders) {

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

		// generate random riders
		//for (int i = 0; i < 100000; i++) {
		//	currentRider = RiderInfo();
		//	currentRider.currentSpeed = ofRandom(1, 45);
		//	currentRider.distanceTravelled = ofRandom(20, 10000);
		//	updateRiderInfo();
		//	allRiderInfo.push_back(currentRider);
		//}
		//
		///// sort and save rider info
		//std::sort(allRiderInfo.begin(), allRiderInfo.end(), RiderRankFunction);

		Serializer.loadClass(ofToDataPath("configs/AllRiderInfo" + string(CONFIG_TYPE)), (allRiderInfo), ARCHIVE_BINARY);
	}

	startThread();

}

//--------------------------------------------------------------
void BicycleController::setDefaults() {
	ofLogNotice() << className << ": setDefaults";

	nextSensorMode = SENSOR_SIMULATE;

	wheelDiameter = 678; // in millimetres
	updateVelocityTime = 250; // in millis

	velocityDecay = 0.0;
	velocityEase = 0.5;
	velocityNormalSpeed = 20.0;

	riderInactiveTime = 6200; //millis

}

//--------------------------------------------------------------
void BicycleController::update() {

	if (!bUse) return;

	// check thread safe GUI changes
	lock();

	// check what mode we're in
	if (nextSensorMode != currentSensorMode) {
		changeMode();
	}

	unlock();

	// doing all velocity updates in threadedUpdate for now

	// lock the model
	// copy to the appModel
	// unlock the model

}

//--------------------------------------------------------------
void BicycleController::changeMode() {

	// shutdown the current mode

	ofLogNotice() << "Shutting down: " << sensorModes[currentSensorMode];

	switch (currentSensorMode) {
	case SENSOR_TEENSY:
	{

	}
	break;
	case SENSOR_GPIO:
	{
#ifndef TARGET_WIN32
        gpio17.unexport_gpio();
#endif
	}
	break;
	}

	// setup for the next mode

	ofLogNotice() << "Setting up: " << sensorModes[nextSensorMode];

	switch (nextSensorMode) {
	case SENSOR_TEENSY:
	{

	}
	break;
	case SENSOR_GPIO:
	{
#ifndef TARGET_WIN32
        gpio17.setup("17");
        gpio17.export_gpio();
        gpio17.setdir_gpio("in");

        lastMsg = "0";
#endif
	}
	break;
	}

	// reset timeouts
	lastSensorTimeout = ofGetElapsedTimeMillis();
	lastVelocityTimeout = ofGetElapsedTimeMillis();
	lastMeasuredVelocity = 0;

	// set current to next mode
	currentSensorMode = nextSensorMode;
}

//--------------------------------------------------------------
void BicycleController::threadedFunction() {

	while (isThreadRunning()) {

		if (bUse) {

			lock();

			// grab the current mode
			int thisCurrentMode = currentSensorMode;

			// calculate the amount of time since the last sensor measurement
			timeSinceLastSensor = ofGetElapsedTimeMillis() - lastSensorTimeout;

			unlock();



			switch (thisCurrentMode) {
			case SENSOR_SIMULATE:
			{
				double wheelCircumference = wheelDiameter * PI;
				double targetVelocity = simulateVelocity * 1000.0 * 1000.0 / 60.0 / 60.0; // km/h * meters * millimeters / minutes / seconds = mm/s
				double simulateTimeout = wheelCircumference / targetVelocity * 1000.0; // mm / mm/s * 1000.0 = milliseconds
				if (timeSinceLastSensor >= simulateTimeout) {
					triggerSensor(SENSOR_SIMULATE);
				}
			}
			break;
			case SENSOR_TEENSY:
			{

			}
			break;
			case SENSOR_GPIO:
			{
#ifndef TARGET_WIN32
                // read gpio value
                gpio17.getval_gpio(gio17_state);
                // can we get analogue or only digital values?

                // if some gpio value, then triggerSensor(SENSOR_GPIO)
                if(gio17_state == "0" && lastMsg == "1"){
                    triggerSensor(SENSOR_GPIO);
                }
                lastMsg = gio17_state;
#endif
			}
			break;
			}



			lock();

			// ease currentVelocity - both ease toward zero AND ease toward last measured velocity
			if (ofGetElapsedTimeMillis() - lastVelocityTimeout > updateVelocityTime) {
				currentAverageVelocity = currentAverageVelocity * (1.0 - velocityEase) + lastMeasuredVelocity * velocityEase;
				currentNormalisedVelocity = currentAverageVelocity / velocityNormalSpeed;
				lastMeasuredVelocity = lastMeasuredVelocity - velocityDecay; // do we need this?
				
				// update rider info
				currentRider.currentSpeed = currentAverageVelocity;
				currentRider.normalisedSpeed = currentNormalisedVelocity;
				if (bRecordRiders) updateRiderInfo();

				if (lastMeasuredVelocity < 0.0) lastMeasuredVelocity = 0;
				lastVelocityTimeout = ofGetElapsedTimeMillis();
			}

			// check if rider is inactive
			if (ofGetElapsedTimeMillis() - lastSensorTimeout > riderInactiveTime) {
				if (bIsRiderActive) {
					bIsRiderActive = false;
					currentAverageVelocity = lastMeasuredVelocity = 0.0;

					if (bRecordRiders) {
						allRiderInfo.push_back(currentRider);

						/// sort and save rider info
						std::sort(allRiderInfo.begin(), allRiderInfo.end(), RiderRankFunction);

						for (int i = 0; i < allRiderInfo.size(); i++) {
							allRiderInfo[i].ranking = i;
							//cout << allRiderInfo[i].ranking << " " << allRiderInfo[i].topSpeed << " " << allRiderInfo[i].distanceTravelled << endl;
						}

						currentRider = RiderInfo(); // reset current rider

						Serializer.saveClass(ofToDataPath("configs/AllRiderInfo" + string(CONFIG_TYPE)), (allRiderInfo), ARCHIVE_BINARY);
					}
					

					ofLogVerbose() << "Rider Inactive";
				}
			}

			unlock();

		}

		ofSleepMillis(1);

	}

}

//--------------------------------------------------------------
void BicycleController::updateRiderInfo() {
	bool bUpdateTop = false;
	if (currentRider.currentSpeed > currentRider.topSpeed) {
		bUpdateTop = true;
		currentRider.topSpeed = currentRider.currentSpeed;
	}

	currentRider.ranking = allRiderInfo.size();
	for (int i = 0; i < allRiderInfo.size(); i++) {
		if (currentRider.topSpeed > allRiderInfo[i].topSpeed) {
			currentRider.ranking = i;
			break;
		}
	}

	currentRider.currentAnimal = "";
	for (int i = 0; i < milestonesSpeed.size(); i++) {
		if (currentRider.currentSpeed < milestonesSpeed[i].value) {
			if (i > 0) {
				currentRider.currentAnimal = milestonesSpeed[i].type;
			}
			break;
		}
	}

	currentRider.currentDevice = "";
	for (int i = 0; i < milestonesWatts.size(); i++) {
		if (currentRider.currentKiloWatts < milestonesSpeed[i].value) {
			if (i > 0) {
				currentRider.currentDevice = milestonesSpeed[i].type;
			}
			break;
		}
	}

	if (bUpdateTop) {
		currentRider.topAnimal = currentRider.currentAnimal;
		currentRider.topDevice = currentRider.currentDevice;
	}

}

//--------------------------------------------------------------
void BicycleController::triggerSensor(SensorMode sensorMode) {

	lock();

	if (sensorMode == currentSensorMode) {

		if (!bIsRiderActive) {
			bIsRiderActive = true;
			distanceTravelled = 0;
			currentRider.isActive = bIsRiderActive;
			ofLogVerbose() << "Rider Active";
		}

		lastSensorTimeout = ofGetElapsedTimeMillis();
		lastMeasuredVelocity = (wheelDiameter * PI / 1000.0 / 1000.0) / (timeSinceLastSensor / 1000.0 / 60.0 / 60.0);
		distanceTravelled += (wheelDiameter * PI / 1000.0);
		currentRider.distanceTravelled += distanceTravelled;
	}

	unlock();

}

//--------------------------------------------------------------
void BicycleController::setRecordRiders(bool b) {
	ofScopedLock lock(mutex);
	bRecordRiders = b;
}

//--------------------------------------------------------------
const RiderInfo & BicycleController::getCurrentRiderInfo(){
	ofScopedLock lock(mutex);
	return currentRider;
}

//--------------------------------------------------------------
void BicycleController::drawGUI() {

	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{

			ImGui::SliderInt("Wheel Diameter (mm)", &wheelDiameter, 600, 700);

			ImGui::SliderInt("Update Velocity (millis)", &updateVelocityTime, 20, 1000);

			ImGui::SliderFloat("Velocity Decay (per/update)", &velocityDecay, 0.0, 20.0);
			ImGui::SliderFloat("Velocity Ease (per/update)", &velocityEase, 0.01, 1.0);

			ImGui::SliderInt("Rider Inactive Time (millis)", &riderInactiveTime, 5000, 30000);

			ImGui::SliderFloat("Velocity Normal (km/h)", &velocityNormalSpeed, 0.01, 60.0);

			ImGui::Combo("Sensor Mode", (int*)&nextSensorMode, sensorModes);

			switch (nextSensorMode) {
			case SENSOR_SIMULATE:
			{
				ImGui::SliderInt("Target Speed", &simulateVelocity, 0, 60);
			}
			break;
			case SENSOR_TEENSY:
			{

			}
			break;
			case SENSOR_GPIO:
			{

			}
			break;
			}

			lock();

			ImGui::NewLine();
			ImGui::Text("Current average velocity %.3f km/hour", currentAverageVelocity);
			ImGui::Text("Current normalised velocity %.3f", currentNormalisedVelocity);
			ImGui::Text("Last measured velocity %.3f km/hour", lastMeasuredVelocity);
			ImGui::Text("Time since last sensor reading : %.0f millis", timeSinceLastSensor);
			

			if (bRecordRiders) {
				ImGui::NewLine();
				for (int i = 0; i < MIN(allRiderInfo.size(), 5); i++) {
					ImGui::Text("%i  %.3f km/hr  %.3f m  %s  %s", allRiderInfo[i].ranking + 1, allRiderInfo[i].topSpeed, allRiderInfo[i].distanceTravelled, allRiderInfo[i].topAnimal, allRiderInfo[i].topDevice);
				}
			}

			unlock();

		}
		endGUI();
	}
}

//--------------------------------------------------------------
bool BicycleController::loadParameters() {
	return Serializer.loadClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool BicycleController::saveParameters() {
	return Serializer.saveClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
