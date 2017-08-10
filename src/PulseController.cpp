#include "PulseController.h"

/* Portion pertaining to Pan-Tompkins QRS detection */

#define M 			50
#define N 			300
#define winSize			250 
#define HP_CONSTANT		((float) 1 / (float) M)
#define MAX_BPM  		200
#define RAND_RES 100000000

// circular buffer for input ecg signal
// we need to keep a history of M + 1 samples for HP filter
float ecg_buff[M + 1] = { 0 };
int ecg_buff_WR_idx = 0;
int ecg_buff_RD_idx = 0;

// circular buffer for input ecg signal
// we need to keep a history of N+1 samples for LP filter
float hp_buff[N + 1] = { 0 };
int hp_buff_WR_idx = 0;
int hp_buff_RD_idx = 0;

// LP filter outputs a single point for every input point
// This goes straight to adaptive filtering for eval
float next_eval_pt = 0;

// running sums for HP and LP filters, values shifted in FILO
float hp_sum = 0;
float lp_sum = 0;

// working variables for adaptive thresholding
float treshold = 0;
bool triggered = false;
int trig_time = 0;
float win_max = 0;
int win_idx = 0;

// numebr of starting iterations, used determine when moving windows are filled
int number_iter = 0;



int tmp = 0;


static bool detect(float new_ecg_pt) {
	// copy new point into circular buffer, increment index
	ecg_buff[ecg_buff_WR_idx++] = new_ecg_pt;
	ecg_buff_WR_idx %= (M + 1);


	/* High pass filtering */
	if (number_iter < M) {
		// first fill buffer with enough points for HP filter
		hp_sum += ecg_buff[ecg_buff_RD_idx];
		hp_buff[hp_buff_WR_idx] = 0;
	}
	else {
		hp_sum += ecg_buff[ecg_buff_RD_idx];

		tmp = ecg_buff_RD_idx - M;
		if (tmp < 0) tmp += M + 1;

		hp_sum -= ecg_buff[tmp];

		float y1 = 0;
		float y2 = 0;

		tmp = (ecg_buff_RD_idx - ((M + 1) / 2));
		if (tmp < 0) tmp += M + 1;

		y2 = ecg_buff[tmp];

		y1 = HP_CONSTANT * hp_sum;

		hp_buff[hp_buff_WR_idx] = y2 - y1;
	}

	// done reading ECG buffer, increment position
	ecg_buff_RD_idx++;
	ecg_buff_RD_idx %= (M + 1);

	// done writing to HP buffer, increment position
	hp_buff_WR_idx++;
	hp_buff_WR_idx %= (N + 1);


	/* Low pass filtering */

	// shift in new sample from high pass filter
	lp_sum += hp_buff[hp_buff_RD_idx] * hp_buff[hp_buff_RD_idx];

	if (number_iter < N) {
		// first fill buffer with enough points for LP filter
		next_eval_pt = 0;

	}
	else {
		// shift out oldest data point
		tmp = hp_buff_RD_idx - N;
		if (tmp < 0) tmp += (N + 1);

		lp_sum -= hp_buff[tmp] * hp_buff[tmp];

		next_eval_pt = lp_sum;
	}

	// done reading HP buffer, increment position
	hp_buff_RD_idx++;
	hp_buff_RD_idx %= (N + 1);


	/* Adapative thresholding beat detection */
	// set initial threshold				
	if (number_iter < winSize) {
		if (next_eval_pt > treshold) {
			treshold = next_eval_pt;
		}

		// only increment number_iter iff it is less than winSize
		// if it is bigger, then the counter serves no further purpose
		number_iter++;
	}

	// check if detection hold off period has passed
	if (triggered == true) {
		trig_time++;

		if (trig_time >= 100) {
			triggered = false;
			trig_time = 0;
		}
	}

	// find if we have a new max
	if (next_eval_pt > win_max) win_max = next_eval_pt;

	// find if we are above adaptive threshold
	if (next_eval_pt > treshold && !triggered) {
		triggered = true;

		return true;
	}
	// else we'll finish the function before returning FALSE,
	// to potentially change threshold

	// adjust adaptive threshold using max of signal found 
	// in previous window            
	if (win_idx++ >= winSize) {
		// weighting factor for determining the contribution of
		// the current peak value to the threshold adjustment
		float gamma = 0.175;

		// forgetting factor - 
		// rate at which we forget old observations
		// choose a random value between 0.01 and 0.1 for this, 
		float alpha = 0.01 + (((float)ofRandom(0, RAND_RES) / (float)(RAND_RES)) * ((0.1 - 0.01)));

		// compute new threshold
		treshold = alpha * gamma * win_max + (1 - alpha) * treshold;

		// reset current window index
		win_idx = 0;
		win_max = -10000000;
	}

	// return false if we didn't detect a new QRS
	return false;

}

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
	sensorData.resize(ofGetWidth());
	currentSensorIndex = 0;

	startThread();

}

//--------------------------------------------------------------
void PulseController::setDefaults() {
	ofLogNotice() << className << ": setDefaults";

	nextSensorMode = SENSOR_SIMULATE;

	targetPulseRate = 80.0f;

	pulseRecvDelay = 20;

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
#ifndef TARGET_WIN32
		system("stty raw crtscts -F /dev/ttyACM0");
#endif
		serial.enumerateDevices();
		serial.setup(0, 115200);
		// on PI need to execute stty raw crtscts -F /dev/ttyACM0
		// need to insert this in .bash_profile
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
						lock();
						int recv = serial.readBytes((unsigned char*)&pdu.chars[0], sizeof(PulseData));
						sensorData[currentSensorIndex] = pdu.data.signal;
						currentSensorIndex++;
						if (currentSensorIndex >= sensorData.size()) currentSensorIndex = 0;
						unlock();
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
				bool b = detect((float)pdu.data.signal / 1024.0f * 4.0);
				ImGui::Text("Pulse QRS: %i", b * 1000);

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
