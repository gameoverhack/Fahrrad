#include "BicycleController.h"

bool RiderRankFunction(const RiderInfo& a, const RiderInfo& b) { 
	//return	a.month > b.month ||
	//		a.month == b.month && a.day > b.day ||
	//		a.month == b.month && a.day == b.day && a.topSpeed > b.topSpeed;
	return a.topSpeed > b.topSpeed;
}

//--------------------------------------------------------------
BicycleController::BicycleController() {
	className = "BicycleController";
	ofLogNotice() << className << ": constructor";
}

//--------------------------------------------------------------
BicycleController::~BicycleController() {
	ofLogNotice() << className << ": destructor";

	// kill the thread

	//lock();
	//delete[] riderSummary.data;
	//unlock();

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

		totalDistanceTravelled = 0;

		startDay = 30;
		startMonth = 5 - 1;
		startYear = 2017;

		endDay = 1;
		endMonth = 5 - 1;
		endYear = 2018;

		//int totalDays = days_between(startDay, startMonth, startYear, endDay, endMonth, endYear);
		
		bIsDataLoaded = false;
		bDay = startDay;
		bMonth = startMonth;
		bYear = startYear;

		numTopRiders = 3;
		topRiderInfo.resize(numTopRiders + 2); // top N + current + last

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
	boosterDifficulty = 5;
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

			if (bRecordRiders) {

				if (!bIsDataLoaded) {

					bool bDone = false;
					int nMonth = bMonth + 1;
					int nYear = bYear;
					if (nMonth > 11) {
						nMonth = 0;
						nYear = bYear + 1;
					}
					int nDay = 1;
					if (nYear == endYear && bMonth == endMonth) {
						nDay = endDay;
					}
					if (nYear == endYear && bMonth == endMonth && nDay == endDay) {
						nMonth = bMonth;
						bDone = true;
					}

					int daysInMonth = days_between(bDay, bMonth, bYear, nDay, nMonth, nYear);

					if (daysInMonth > 0) {

						ofLogNotice() << "Adding rider info: " << daysInMonth << " " << bDay << " " << bMonth << " " << bYear << " " << nDay << " " << nMonth << " " << nYear << endl;
						
						int month = bMonth + 1;
						int year = bYear;
						for (int day = bDay; day < bDay + daysInMonth; day++) {

							vector<RiderInfo> todaysRiderInfo;

							ostringstream os;
							os << year << "_" << std::setfill('0') << std::setw(2) << month << "_" << std::setfill('0') << std::setw(2) << day;


							//for (int i = 0; i < 1000; i++) {
							//	// generate random riders
							//	currentRider = RiderInfo();
							//	currentRider.currentSpeed = ofRandom(1, 58);
							//	currentRider.time = ofRandom(0.5 * 60 * 1000, 10 * 60 * 1000);
							//	currentRider.distanceTravelled = currentRider.currentSpeed / 60.0 / 60.0 / 1000.0 * currentRider.time;
							//	currentRider.day = day;
							//	currentRider.month = month;
							//	currentRider.year = year;
							//	updateRiderInfo();
							//	todaysRiderInfo.push_back(currentRider);
							//}
							//Serializer.saveClass(ofToDataPath("configs/stats/dailyRiderInfo_" + os.str() + string(CONFIG_TYPE)), (todaysRiderInfo), ARCHIVE_BINARY);


							ofLogNotice() << "Loading day: " << os.str();
							Serializer.loadClass(ofToDataPath("configs/stats/dailyRiderInfo_" + os.str() + string(CONFIG_TYPE)), (todaysRiderInfo), ARCHIVE_BINARY);


							float dailyDistance = 0;
							for (int rider = 0; rider < todaysRiderInfo.size(); rider++) {
								
								dailyDistance += todaysRiderInfo[rider].distanceTravelled;
								totalTimeTaken += todaysRiderInfo[rider].time;
								totalDistanceTravelled += todaysRiderInfo[rider].distanceTravelled;
								allRiderInfo.push_back(todaysRiderInfo[rider]);

							}

							totalNumberRiders += todaysRiderInfo.size();
							totalDailyDistances.push_back(dailyDistance);

							int dayOfWeek = day_of_week(day, month, year);
							daysOfWeek.push_back(dayOfWeek);
							bool bUseDay = false;
							for (int i = 0; i < daysOpen.size(); i++) {
								if (daysOpen[i] == dayOfWeek) {
									bUseDay = true;
									activeDaysUsed++;
									break;
								}
							}
							daysOfWeekToUse.push_back(bUseDay);

							dailyRiderInfo[os.str()] = todaysRiderInfo;

						}

					}

					bDay = 1;
					bMonth = nMonth;
					bYear = nYear;

					if (bDone) {

						//for (unordered_map< string, vector<RiderInfo> >::iterator it = dailyRiderInfo.begin(); it != dailyRiderInfo.end(); ++it) {
						//	ofLogNotice() << "Check: " << it->first << " with " << it->second.size() << " riders";
						//}

						// sort todays rider info

						vector<RiderInfo>& todaysRiderInfo = getTodaysRiderInfo();
						std::sort(todaysRiderInfo.begin(), todaysRiderInfo.end(), RiderRankFunction);
						std::sort(allRiderInfo.begin(), allRiderInfo.end(), RiderRankFunction);

						int dayranking = 0;
						for (int i = 0; i < todaysRiderInfo.size(); i++) {
							todaysRiderInfo[i].dayranking = dayranking;
							dayranking++;
						}

						int allranking = 0;
						for (int i = 0; i < allRiderInfo.size(); i++) {
							allRiderInfo[i].allranking = allranking;
							if (i < numTopRiders) topRiderInfo[i] = allRiderInfo[i];
							allranking++;
						}

						topRiderInfo[topRiderInfo.size() - 1] = allRiderInfo[allRiderInfo.size() - 1];

						bIsDataLoaded = true;

						//riderSummary.data = new float[4 + totalDailyDistances.size()];
						riderSummary.data = new float[RS_DATA_START + activeDaysUsed];
						riderSummary.data[RS_DATA_SIZE] = RS_DATA_START + activeDaysUsed;

					}
				}
			}

			// ease currentVelocity - both ease toward zero AND ease toward last measured velocity
			if (ofGetElapsedTimeMillis() - lastVelocityTimeout > updateVelocityTime) {
				currentAverageVelocity = currentAverageVelocity * (1.0 - velocityEase) + lastMeasuredVelocity * velocityEase;
				currentNormalisedVelocity = currentAverageVelocity / velocityNormalSpeed;
				lastMeasuredVelocity = lastMeasuredVelocity - velocityDecay; // do we need this?
				
				// update rider info
				currentRider.currentSpeed = currentAverageVelocity;
				currentRider.normalisedSpeed = currentNormalisedVelocity;
				currentRider.currentKiloWatts = maxWatts[boosterDifficulty] * currentRider.currentSpeed;
				
				if (bRecordRiders && bIsDataLoaded) {

					if (bIsRiderActive) updateRiderInfo();

					topRiderInfo[topRiderInfo.size() - 2] = currentRider;

					riderSummary.data[RS_SPEED_CURRENT] = currentRider.currentSpeed;
					riderSummary.data[RS_SPEED_ANIMAL] = currentRider.currentAnimal;
					riderSummary.data[RS_DISTANCE_DAY] = totalDailyDistances[getTodaysRiderIndex()];
					riderSummary.data[RS_DISTANCE_TOTAL] = totalDistanceTravelled;
					riderSummary.data[RS_TIME_TOTAL] = (totalTimeTaken + currentRider.time) / 1000.0f / 60.0f / 60.0f;
					riderSummary.data[RS_RIDERS_TOTAL] = totalNumberRiders;

					int usedDay = 0;

					for (int i = 0; i < totalDailyDistances.size(); i++) {
						if (daysOfWeekToUse[i]) {
							riderSummary.data[RS_DATA_START + usedDay] = totalDailyDistances[i];
							usedDay++;
						}
					}

				}

				if (lastMeasuredVelocity < 0.0) lastMeasuredVelocity = 0;
				lastVelocityTimeout = ofGetElapsedTimeMillis();
			}

			// check if rider is inactive
			if (ofGetElapsedTimeMillis() - lastSensorTimeout > riderInactiveTime) {
				
				if (bIsRiderActive) {

					bIsRiderActive = false;
					currentAverageVelocity = lastMeasuredVelocity = 0.0;

					if (bRecordRiders && bIsDataLoaded) {

						vector<RiderInfo>& todaysRiderInfo = getTodaysRiderInfo();

						currentRider.isActive = false;

						totalNumberRiders++;
						totalTimeTaken += currentRider.time;

						todaysRiderInfo.push_back(currentRider);
						allRiderInfo.push_back(currentRider);
						currentRider = RiderInfo(); // reset current rider

						// sort and save rider info
						std::sort(todaysRiderInfo.begin(), todaysRiderInfo.end(), RiderRankFunction);
						std::sort(allRiderInfo.begin(), allRiderInfo.end(), RiderRankFunction);

						int dayranking = 0;
						for (int i = 0; i < todaysRiderInfo.size(); i++) {
							todaysRiderInfo[i].dayranking = dayranking;
							dayranking++;
						}

						int allranking = 0;
						for (int i = 0; i < allRiderInfo.size(); i++) {
							allRiderInfo[i].allranking = allranking;
							if (i < numTopRiders) topRiderInfo[i] = allRiderInfo[i];
							allranking++;
						}

						topRiderInfo[topRiderInfo.size() - 1] = allRiderInfo[allRiderInfo.size() - 1];

						ostringstream os;
						os << "configs/stats/dailyRiderInfo_" << ofGetYear() << "_" << std::setfill('0') << std::setw(2) << ofGetMonth() << "_" << std::setfill('0') << std::setw(2) << ofGetDay() << string(CONFIG_TYPE);
						Serializer.saveClass(ofToDataPath(os.str()), (todaysRiderInfo), ARCHIVE_BINARY);
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
int BicycleController::getTodaysRiderIndex() {
	return days_between(startDay, startMonth, startYear, ofGetDay(), ofGetMonth() - 1, ofGetYear());
}

//--------------------------------------------------------------
vector<RiderInfo>& BicycleController::getTodaysRiderInfo(){
	ostringstream os;
	os << ofGetYear() << "_" << std::setfill('0') << std::setw(2) << ofGetMonth() << "_" << std::setfill('0') << std::setw(2) << ofGetDay();
	return dailyRiderInfo[os.str()];
}

//--------------------------------------------------------------
void BicycleController::updateRiderInfo() {

	if (currentRider.currentSpeed > currentRider.topSpeed) {
		currentRider.topSpeed = currentRider.currentSpeed;
	}

	currentRider.dayranking = 0;

	vector<RiderInfo>& todaysRiderInfo = getTodaysRiderInfo();

	for (int i = 0; i < todaysRiderInfo.size(); i++) {
		if (currentRider.topSpeed > todaysRiderInfo[i].topSpeed) {
			break;
		}
		else {
			currentRider.dayranking++;
		}
	}

	currentRider.allranking = 0;

	for (int i = 0; i < allRiderInfo.size(); i++) {
		if (currentRider.topSpeed > allRiderInfo[i].topSpeed) {
			break;
		}
		else {
			currentRider.allranking++;
		}
	}

	currentRider.currentAnimal = -1;
	for (int i = 0; i < milestonesSpeed.size(); i++) {
		if (currentRider.currentSpeed < milestonesSpeed[i].value) {
			if (i > 0) {
				currentRider.currentAnimal = i;//milestonesSpeed[i].type;
			}
			break;
		}
	}

	currentRider.currentDevice = -1;
	for (int i = 0; i < milestonesWatts.size(); i++) {
		if (currentRider.currentKiloWatts < milestonesWatts[i].value) {
			if (i > 0) {
				currentRider.currentDevice = i;//milestonesSpeed[i].type;
			}
			break;
		}
	}

	currentRider.time = ofGetElapsedTimeMillis() - riderStartTimeMillis;

}

//--------------------------------------------------------------
void BicycleController::triggerSensor(SensorMode sensorMode) {

	lock();

	if (bRecordRiders && !bIsDataLoaded) {
		unlock();
		return;
	}

	if (sensorMode == currentSensorMode) {

		if (!bIsRiderActive) {
			bIsRiderActive = true;
			distanceTravelled = 0;
			currentRider.isActive = bIsRiderActive;
			riderStartTimeMillis = ofGetElapsedTimeMillis();
			currentRider.day = ofGetDay();
			currentRider.month = ofGetMonth();
			currentRider.year = ofGetYear();
			ofLogVerbose() << "Rider Active";
		}

		lastSensorTimeout = ofGetElapsedTimeMillis();
		float dist = (wheelDiameter * PI / 1000.0);
		lastMeasuredVelocity = (dist / 1000.0) / (timeSinceLastSensor / 1000.0 / 60.0 / 60.0);
		distanceTravelled += dist;
		currentRider.distanceTravelled += dist;
		totalDistanceTravelled += dist;
		if(bRecordRiders) totalDailyDistances[getTodaysRiderIndex()] += dist;

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
const vector<RiderInfo>& BicycleController::getTopRiderInfo() {
	ofScopedLock lock(mutex);
	return topRiderInfo;
}

//--------------------------------------------------------------
const RiderSummaryUnion & BicycleController::getRiderSummary() {
	ofScopedLock lock(mutex);
	return riderSummary;
}

//--------------------------------------------------------------
bool BicycleController::isDataLoaded() {
	ofScopedLock lock(mutex);
	return bIsDataLoaded;
}

//--------------------------------------------------------------
string BicycleController::getAnimalFromIndex(const int & index) {
	if (index >= 0 && index < milestonesSpeed.size()) {
		return milestonesSpeed[index].type;
	} else {
		return "";
	}
}

//--------------------------------------------------------------
string BicycleController::getDeviceFromIndex(const int & index) {
	if (index >= 0 && index < milestonesWatts.size()) {
		return milestonesWatts[index].type;
	}
	else {
		return "";
	}
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

			ImGui::SliderInt("Rider Inactive Time (millis)", &riderInactiveTime, 1000, 6000);

			ImGui::SliderFloat("Velocity Normal (km/h)", &velocityNormalSpeed, 0.01, 60.0);

			ImGui::SliderInt("Booster Difficulty", &boosterDifficulty, 0, maxWatts.size() - 1);
			
			ImGui::Combo("Sensor Mode", (int*)&nextSensorMode, sensorModes);

			switch (nextSensorMode) {
			case SENSOR_SIMULATE:
			{
				ImGui::SliderInt("Target Speed", &simulateVelocity, 0, 75);
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
				ImGui::Text("Total distanc travelled: %.3f m", totalDistanceTravelled);
				ImGui::NewLine();
				int numRequested = 5;
				int numFound = 0;
				vector<RiderInfo>& todaysRiderInfo = getTodaysRiderInfo();
				for (int i = 0; i < todaysRiderInfo.size(); i++) {
					if (numFound < numRequested) {
						ImGui::Text("day: %i  %.3f km/hr  %.3f m  %s  %s %i / %i / %i", todaysRiderInfo[i].dayranking + 1,
							todaysRiderInfo[i].topSpeed, todaysRiderInfo[i].distanceTravelled,
							getAnimalFromIndex(todaysRiderInfo[i].currentAnimal).c_str(), getDeviceFromIndex(todaysRiderInfo[i].currentDevice).c_str(),
							todaysRiderInfo[i].day, todaysRiderInfo[i].month, todaysRiderInfo[i].year);
						numFound++;
					}
				}
				ImGui::NewLine();
				numFound = 0;
				for (int i = 0; i < allRiderInfo.size(); i++) {
					if (numFound < numRequested) {
						ImGui::Text("all: %i  %.3f km/hr  %.3f m  %s  %s %i / %i / %i", allRiderInfo[i].allranking + 1,
							allRiderInfo[i].topSpeed, allRiderInfo[i].distanceTravelled,
							getAnimalFromIndex(allRiderInfo[i].currentAnimal).c_str(), getDeviceFromIndex(allRiderInfo[i].currentDevice).c_str(),
							allRiderInfo[i].day, allRiderInfo[i].month, allRiderInfo[i].year);
						numFound++;
					}
				}
				ImGui::NewLine();
				int index = getTodaysRiderIndex();
				ImGui::Text("todays index: %i distance: %.3f m total days: %i  used days: %i data: %i bytes", index, totalDailyDistances[getTodaysRiderIndex()], totalDailyDistances.size(), activeDaysUsed, (4 + activeDaysUsed)*sizeof(float));

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
