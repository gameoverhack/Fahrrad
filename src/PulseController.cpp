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

	startThread();

}

//--------------------------------------------------------------
void PulseController::setDefaults() {
	ofLogNotice() << className << ": setDefaults";

	nextSensorMode = SENSOR_SIMULATE;

	targetPulseRate = 80.0f;

	pulseRecvDelay = 15;

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
	case SENSOR_TEENSY:
	{
		serial.close();
	}
	break;
	case SENSOR_GPIO:
	{
//#ifndef TARGET_WIN32
//        gpio17.unexport_gpio();
//#endif
	}
	break;
	}

	// setup for the next mode

	ofLogNotice() << "Setting up: " << sensorModes[nextSensorMode];

	switch (nextSensorMode) {
	case SENSOR_TEENSY:
	{
		serial.enumerateDevices();
		serial.setup(0, 115200);
	}
	break;
	case SENSOR_GPIO:
	{
//#ifndef TARGET_WIN32
//        gpio17.setup("17");
//        gpio17.export_gpio();
//        gpio17.setdir_gpio("in");
//
//        lastMsg = "0";
//#endif
	}
	break;
	}

	// reset timeouts
	lastSensorTimeout = ofGetElapsedTimeMillis();

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

			// calculate the amount of time since the last sensor measurement
			timeSinceLastSensor = ofGetElapsedTimeMillis() - lastSensorTimeout;

			unlock();



			switch (thisCurrentMode) {
			case SENSOR_SIMULATE:
			{
				lock();
				pdu.data.bpm = ofRandom(CLAMP(targetPulseRate - 20, 0, 220), CLAMP(targetPulseRate + 20, 0, 220));
				pdu.data.signal = sin(ofGetElapsedTimef()) * 200 + 400;
				pdu.data.beat = sin(ofGetElapsedTimef()) == 1 ? 1000 : 0;
				unlock();
			}
			break;
			case SENSOR_TEENSY:
			{
				if (serial.available()) {
					if (serial.readByte() == 255) {
						int recv = serial.readBytes((unsigned char*)&pdu.chars[0], sizeof(PulseData));
						if (recv != sizeof(PulseData)) {
							ofLogError() << "pulse recv err: " << recv << " / " << sizeof(PulseData) << endl;
						}
						else {
							//calculateBPM();
						}
						ofSleepMillis(pulseRecvDelay);
					}
				}
			}
			break;
			case SENSOR_GPIO:
			{
//#ifndef TARGET_WIN32
//                // read gpio value
//                gpio17.getval_gpio(gio17_state);
//                // can we get analogue or only digital values?
//
//                // if some gpio value, then triggerSensor(SENSOR_GPIO)
//                if(gio17_state == "0" && lastMsg == "1"){
//                    triggerSensor(SENSOR_GPIO);
//                }
//                lastMsg = gio17_state;
//#endif
			}
			break;
			}

		}

		ofSleepMillis(1);

	}

}

////--------------------------------------------------------------
//void PulseController::calculateBPM() {
//
//	timeSinceLastSignal = ofGetElapsedTimeMillis() - timeAtLastSignal;
//	timeAtLastSignal = ofGetElapsedTimeMillis();
//
//	sampleCounter += timeSinceLastSignal;
//
//	int N = sampleCounter - lastBeatTime;
//	int Signal = pdu.data.signal;
//	if (Signal < thresh && N >(IBI / 5) * 3) {       // avoid dichrotic noise by waiting 3/5 of last IBI
//		if (Signal < T) {												// T is the trough
//			T = Signal;												// keep track of lowest point in pulse wave
//		}
//	}
//
//	if (Signal > thresh && Signal > P) {						// thresh condition helps avoid noise
//		P = Signal;													// P is the peak
//	}																			// keep track of highest point in pulse wave
//
//																				//  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
//																				// signal surges up in value every time there is a pulse
//	if (N > 250) {												// avoid high frequency noise
//		if ((Signal > thresh) && (Pulse == false) && (N > (IBI / 5) * 3)) {
//			Pulse = true;                               // set the Pulse flag when we think there is a pulse
//			IBI = sampleCounter - lastBeatTime;         // measure time between beats in mS
//			lastBeatTime = sampleCounter;               // keep track of time for next pulse
//
//			if (secondBeat) {                        // if this is the second beat, if secondBeat == TRUE
//				secondBeat = false;                  // clear secondBeat flag
//				for (int i = 0; i <= 9; i++) {             // seed the running total to get a realisitic BPM at startup
//					rate[i] = IBI;
//				}
//			}
//
//			if (firstBeat) {                         // if it's the first time we found a beat, if firstBeat == TRUE
//				firstBeat = false;                   // clear firstBeat flag
//				secondBeat = true;                   // set the second beat flag
//				return;                              // IBI value is unreliable so discard it
//			}
//
//
//			// keep a running total of the last 10 IBI values
//			int runningTotal = 0;                  // clear the runningTotal variable
//
//			for (int i = 0; i <= 8; i++) {                // shift data in the rate array
//				rate[i] = rate[i + 1];                  // and drop the oldest IBI value
//				runningTotal += rate[i];              // add up the 9 oldest IBI values
//			}
//
//			rate[9] = IBI;                          // add the latest IBI to the rate array
//			runningTotal += rate[9];                // add the latest IBI to runningTotal
//			runningTotal /= 10;                     // average the last 10 IBI values
//			BPM = 60000 / runningTotal;               // how many beats can fit into a minute? that's BPM!
//			QS = true;                              // set Quantified Self flag
//													// QS FLAG IS NOT CLEARED INSIDE THIS ISR
//		}
//	}
//
//	if (Signal < thresh && Pulse == true) {   // when the values are going down, the beat is over
//		Pulse = false;                         // reset the Pulse flag so we can do it again
//		amp = P - T;                           // get amplitude of the pulse wave
//		thresh = amp / 2 + T;                    // set thresh at 50% of the amplitude
//		P = thresh;                            // reset these for next time
//		T = thresh;
//	}
//
//	if (N > 2500) {                           // if 2.5 seconds go by without a beat
//		thresh = 530;                          // set thresh default
//		P = 512;                               // set P default
//		T = 512;                               // set T default
//		lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date
//		firstBeat = true;                      // set these to avoid noise
//		secondBeat = false;                    // when we get the heartbeat back
//	}
//}

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
			case SENSOR_TEENSY:
			{
				ImGui::SliderInt("Pulse Recv Delay", &pulseRecvDelay, 0, 50);
				ImGui::NewLine();
				lock();
				ImGui::Text("Pulse signal: %i BPM: %i Beat: %i", pdu.data.signal, pdu.data.bpm, pdu.data.beat);
				//if (QS) QS = false;
				unlock();
				
			}
			break;
			case SENSOR_GPIO:
			{

			}
			break;
			}

			//lock();

			//ImGui::NewLine();
			//ImGui::Text("Current average velocity %.3f km/hour", currentAverageVelocity);

			//unlock();

		}
		endGUI();
	}
}

//--------------------------------------------------------------
const PulseData & PulseController::getPulseData() {
	ofScopedLock lock(mutex);
	return pdu.data;
}

//--------------------------------------------------------------
bool PulseController::loadParameters() {
	return Serializer.loadClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool PulseController::saveParameters() {
	return Serializer.saveClass(ofToDataPath("configs/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
