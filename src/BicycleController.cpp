#include "BicycleController.h"

//--------------------------------------------------------------
BicycleController::BicycleController() {
	className = "BicycleController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
BicycleController::~BicycleController() {
	ofLogNotice() << className << ": destructor";
	IGuiBase::~IGuiBase(); // call base destructor
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

	startThread();
	
}

//--------------------------------------------------------------
void BicycleController::setDefaults() {
	ofLogNotice() << className << ": setDefaults";

	nextSensorMode = SENSOR_SIMULATE;

	wheelDiameter = 678; // in millimetres
	updateVelocityTime = 250; // in millis
	simulateVelocity = 0;
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


	// lock speed updates

	// if simulated mode...copy the desired speed to the current speed

	// if in teensy mode ... get the speed from the teensy and copy to current speed

	// if in GPIO mode ... get the speed from the teensy and copy to current speed

	// unlock speed updates

	// lock the model
	// copy to the appModel
	// unlock the model

}

//--------------------------------------------------------------
void BicycleController::changeMode() {

	// shutdown the current mode

	ofLogNotice() << "Shutting down: " << sensorModes[currentSensorMode];

	switch (currentSensorMode) {
	case SENSOR_NONE:
	{
		// nothing
	}
	break;
	case SENSOR_SIMULATE:
	{

	}
	break;
	case SENSOR_KEYBOARD:
	{

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

	// setup for the next mode

	ofLogNotice() << "Setting up: " << sensorModes[nextSensorMode];

	switch (nextSensorMode) {
	case SENSOR_NONE:
	{

	}
	break;
	case SENSOR_SIMULATE:
	{
		lastSensorTimeout = ofGetElapsedTimeMillis();
	}
	break;
	case SENSOR_KEYBOARD:
	{
		lastSensorTimeout = ofGetElapsedTimeMillis();
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

	lastVelocityTimeout = ofGetElapsedTimeMillis();
	lastMeasuredVelocity = 0;
}

//--------------------------------------------------------------
void BicycleController::threadedFunction() {

	while (isThreadRunning()) {

		lock();
		int thisCurrentMode = currentSensorMode;
		
		timeSinceLastSensor = ofGetElapsedTimeMillis() - lastSensorTimeout;
		unlock();

		

		switch (thisCurrentMode) {
		case SENSOR_NONE:
		{

		}
		break;
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
		case SENSOR_KEYBOARD:
		{
			// nothing
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
		// not sure if we will need this when not simulating
		// but basically this eases the measured velocity toward zero
		// and only updates/eases the currentAverage toward last measured
		if (ofGetElapsedTimeMillis() - lastVelocityTimeout > updateVelocityTime) {
			lastMeasuredVelocity = lastMeasuredVelocity - 1.0;
			if (lastMeasuredVelocity < 0.0) lastMeasuredVelocity = 0;
			currentAverageVelocity = currentAverageVelocity * 0.1 + lastMeasuredVelocity * 0.9;
			lastVelocityTimeout = ofGetElapsedTimeMillis();
		}
		
		unlock();

		ofSleepMillis(1);
	}

}

//--------------------------------------------------------------
void BicycleController::triggerSensor(SensorMode sensorMode) {
	lock();
	if (sensorMode == currentSensorMode) {
		lastSensorTimeout = ofGetElapsedTimeMillis();
		lastMeasuredVelocity = (wheelDiameter * PI / 1000.0 / 1000.0) / (timeSinceLastSensor / 1000.0 / 60.0 / 60.0); // should we ease? ie., c * 0.9 + n * 0.1 etc?
	}
	unlock();
}

//--------------------------------------------------------------
double BicycleController::getAverageVelocity() {
	ofScopedLock lock(mutex);
	return currentAverageVelocity;
}

//--------------------------------------------------------------
void BicycleController::drawGUI() {
	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{

			ImGui::SliderInt("Wheel Diameter (mm)", &wheelDiameter, 600, 700);
			ImGui::SliderInt("Update Velocity (millis)", &updateVelocityTime, 20, 1000);
			ImGui::Combo("Sensor Mode", (int*)&nextSensorMode, sensorModes);

			switch (nextSensorMode) {
			case SENSOR_NONE:
			{

			}
			break;
			case SENSOR_SIMULATE:
			{
				ImGui::SliderInt("Target Speed", &simulateVelocity, 0, 60);
			}
			break;
			case SENSOR_KEYBOARD:
			{

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
			ImGui::Text("Current average velocity %.3f km/hour", currentAverageVelocity);
			ImGui::Text("Last measured velocity %.3f km/hour", lastMeasuredVelocity);
			ImGui::Text("Time since last sensor reading : %.0f millis", timeSinceLastSensor);
			
			unlock();

		}
		endGUI();
	}
}

//--------------------------------------------------------------
bool BicycleController::loadParameters() {
	return Serializer.loadClass(ofToDataPath("configs\\" + className + ".conf"), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool BicycleController::saveParameters() {
	return Serializer.saveClass(ofToDataPath("configs\\" + className + ".conf"), (*this), ARCHIVE_BINARY);
}