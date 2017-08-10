#include "PulseController.h"

//--------------------------------------------------------------
PulseController::PulseController() {
	className = "PulseController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
PulseController::~PulseController() {
	ofLogNotice() << className << ": destructor";

	// kill the thread
	waitForThread();

	this->IGuiBase::~IGuiBase(); // call base destructor
}

//--------------------------------------------------------------
void PulseController::setup() {
	ofLogNotice() << className << ": setup";

	// call base clase setup for now
	IGuiBase::setup();

	currentSensorMode = SENSOR_NONE; // so we force sensor mode change in update

	bIsPulseActive = false;
	//timeAtLastSignal = timeSinceLastSignal = ofGetElapsedTimeMillis();
	pulseData.resize(36); // # of marks for pulse display
	rawPulseData.resize(747);

	currentRawPulseDataIndex = 0;
	currentPulseDataIndex = 0;

	startThread();

}

//--------------------------------------------------------------
void PulseController::setDefaults() {

	ofLogNotice() << className << ": setDefaults";

	nextSensorMode = SENSOR_SIMULATE;

	targetPulseRate = 80.0f;

	sensorReadTimeout = 20;
	sensorUpdateTimeout = 500;

}

//--------------------------------------------------------------
void PulseController::update() {

	if (!bUse) return;

	// check thread safe GUI changes
	lock();

	// check what mode we're in
	if (nextSensorMode != currentSensorMode) {
		changeMode();
	}

	unlock();

}

void PulseController::changeMode() {

	// shutdown the current mode

	ofLogNotice() << "Shutting down: " << sensorModes[currentSensorMode];

	switch (currentSensorMode) {
	case SENSOR_ARDUINO:
	{
		serial.close();
	}
	break;
	}

	// setup for the next mode

	ofLogNotice() << "Setting up: " << sensorModes[nextSensorMode];

	switch (nextSensorMode) {
	case SENSOR_ARDUINO:
	{
#ifndef TARGET_WIN32
		// on PI need to execute stty raw crtscts -F /dev/ttyACM0
		// need to insert this in .bash_profile
		system("stty raw crtscts -F /dev/ttyACM0");
#endif
		serial.enumerateDevices();
		serial.setup(0, 115200);
	}
	break;
	}

	// reset timeouts
	lastSensorTimeout = ofGetElapsedTimeMillis();
	lastUpdateTimeout = ofGetElapsedTimeMillis();

	// set current to next mode
	currentSensorMode = nextSensorMode;
}

//--------------------------------------------------------------
void PulseController::threadedFunction() {

	while (isThreadRunning()) {

		if (bUse) {

			lock();

			// grab the current mode
			int thisCurrentMode = currentSensorMode;

			bool bDoSensorRead = (ofGetElapsedTimeMillis() - lastSensorTimeout >= sensorReadTimeout);

			unlock();

			if (bDoSensorRead) {
				lastSensorTimeout = ofGetElapsedTimeMillis();
				switch (thisCurrentMode) {
				case SENSOR_SIMULATE:
				{
					lock();
					
					if (ofGetElapsedTimeMillis() - lastUpdateTimeout >= sensorUpdateTimeout) {
						pdu.data.bpm = ofRandom(CLAMP(targetPulseRate - 20, 0, 220), CLAMP(targetPulseRate + 20, 0, 220));
						pdu.data.signal = 0;//sin(ofGetElapsedTimef()) * 200 + 400;
						pdu.data.beat = 0;//sin(ofGetElapsedTimef()) == 1 ? 1000 : 0;
						lastUpdateTimeout = ofGetElapsedTimeMillis();
					}
					
					unlock();
				}
				break;
				case SENSOR_ARDUINO:
				{
					if (serial.available()) {
						if (serial.readByte() == 255) {
							lock();
							
							int recv = serial.readBytes((unsigned char*)&pdu.chars[0], sizeof(PulseData));
							
							if (recv != sizeof(PulseData)) {
								ofLogError() << "pulse recv err: " << recv << " / " << sizeof(PulseData) << endl;
							}
							else {
								if (ofGetElapsedTimeMillis() - lastUpdateTimeout >= sensorUpdateTimeout) {
									pulseData[currentPulseDataIndex] = pdu.data;
									pulseData[35] = pdu.data;
									currentPulseDataIndex++;
									if (currentPulseDataIndex >= 36) currentPulseDataIndex = 0;
									lastUpdateTimeout = ofGetElapsedTimeMillis();
								}
								rawPulseData[currentRawPulseDataIndex] = pdu.data;
								currentRawPulseDataIndex++;
								if (currentRawPulseDataIndex >= rawPulseData.size()) currentRawPulseDataIndex = 0;
							}
							
							unlock();
							
						}

					}
				}
				break;
				}

			}

			

		}

		ofSleepMillis(1);

	}

}

//--------------------------------------------------------------
void PulseController::drawGUI() {

	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{
			
			ImGui::Combo("Sensor Mode", (int*)&nextSensorMode, sensorModes);

			switch (nextSensorMode) {
			case SENSOR_SIMULATE:
			{
				ImGui::SliderInt("Target Pulse", &targetPulseRate, 0, 200);
			}
			break;
			case SENSOR_ARDUINO:
			{
				ImGui::SliderInt("Sensor Read Timeout", &sensorReadTimeout, 20, 1000);
				ImGui::SliderInt("Sensor Update Timeout", &sensorUpdateTimeout, 20, 1000);
				ImGui::NewLine();
				lock();
				ImGui::Text("Pulse signal: %i BPM: %i Beat: %i", pdu.data.signal, pdu.data.bpm, pdu.data.beat);
				unlock();
				
			}
			break;
			}

		}
		endGUI();
	}
}

//--------------------------------------------------------------
vector<PulseData> PulseController::getPulseData() {
	ofScopedLock lock(mutex);
	return pulseData;
}

//--------------------------------------------------------------
vector<PulseData> PulseController::getRawPulseData() {
	ofScopedLock lock(mutex);
	return rawPulseData;
}

//--------------------------------------------------------------
bool PulseController::loadParameters() {
	return Serializer.loadClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool PulseController::saveParameters() {
	return Serializer.saveClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
