#include "BicycleController.h"

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

		// change sensor mode
		changeMode();

		// set current to next mode
		currentSensorMode = nextSensorMode;
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

	}
	break;
	}

	lastSensorTimeout = ofGetElapsedTimeMillis();
	lastVelocityTimeout = ofGetElapsedTimeMillis();
	lastMeasuredVelocity = 0;

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

			}
			break;
			}



			lock();

			// ease currentVelocity - both ease toward zero AND ease toward last measured velocity
			if (ofGetElapsedTimeMillis() - lastVelocityTimeout > updateVelocityTime) {
				currentAverageVelocity = currentAverageVelocity * (1.0 - velocityEase) + lastMeasuredVelocity * velocityEase;
				currentNormalisedVelocity = currentAverageVelocity / velocityNormalSpeed;
				lastMeasuredVelocity = lastMeasuredVelocity - velocityDecay; // do we need this?
				if (lastMeasuredVelocity < 0.0) lastMeasuredVelocity = 0;
				lastVelocityTimeout = ofGetElapsedTimeMillis();
			}

			// check if rider is inactive
			if (ofGetElapsedTimeMillis() - lastSensorTimeout > riderInactiveTime) {
				if (bIsRiderActive) {
					bIsRiderActive = false;
					currentAverageVelocity = lastMeasuredVelocity = 0.0;
					ofLogVerbose() << "Rider Inactive";
				}
			}

			unlock();

		}

		ofSleepMillis(1);

	}

}

//--------------------------------------------------------------
void BicycleController::triggerSensor(SensorMode sensorMode) {

	lock();

	if (sensorMode == currentSensorMode) {

		if (!bIsRiderActive) {
			bIsRiderActive = true;
			distanceTravelled = 0;
			ofLogVerbose() << "Rider Active";
		}

		lastSensorTimeout = ofGetElapsedTimeMillis();
		lastMeasuredVelocity = (wheelDiameter * PI / 1000.0 / 1000.0) / (timeSinceLastSensor / 1000.0 / 60.0 / 60.0);
		distanceTravelled += (wheelDiameter * PI / 1000.0);
	}

	unlock();

}

//--------------------------------------------------------------
bool BicycleController::getIsRiderActive() {
	ofScopedLock lock(mutex);
	return bIsRiderActive;
}

//--------------------------------------------------------------
double BicycleController::getAverageVelocity() {
	ofScopedLock lock(mutex);
	return currentAverageVelocity;
}

//--------------------------------------------------------------
double BicycleController::getNormalisedVelocity() {
	ofScopedLock lock(mutex);
	return currentNormalisedVelocity;
}

//--------------------------------------------------------------
double BicycleController::getDistanceTravelled() {
	return distanceTravelled;
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

			unlock();

		}
		endGUI();
	}
}

//--------------------------------------------------------------
bool BicycleController::loadParameters() {
	return Serializer.loadClass(fixPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool BicycleController::saveParameters() {
	return Serializer.saveClass(fixPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
