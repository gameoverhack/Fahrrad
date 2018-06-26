#include "BicycleController.h"

bool RiderRankFunction(const RiderInfo& a, const RiderInfo& b) { 
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
		
		totalDistanceTravelled = 0;

		int weekDay = ofGetWeekday();			// 0 == Sunday, 1 == Monday...
		weekDay = ofWrap(weekDay - 1, 0, 7);	// 6 == Sunday, 0 == Monday...

		if (ofGetDay() - weekDay < 1) {
			// we start in the last month
			if (ofGetMonth() == 1) { // handle january
				startMonth = 12 - 1;
				startYear = ofGetYear() - 1;
			}
			else { // handle all other months
				startMonth = ofGetMonth() - 2;
				startYear = ofGetYear();
			}
			startDay = days_in_month(startMonth, startYear) + (ofGetDay() - weekDay) + 1; 

		}
		else {
			// we start in the same month
			startDay = ofGetDay() - weekDay;
			startMonth = ofGetMonth() - 1;
			startYear = ofGetYear();
		}

		endDay = ofGetDay() + 1;
		endMonth = ofGetMonth() - 1;
		endYear = ofGetYear();
		 
		ofLogNotice() << "Start to end dates: " << startDay << " " << startMonth << " " << startYear << "   " << endDay << " " << endMonth << " " << endYear << endl; // remember these are plus and minus one for day and month

		//int daysTillOpening = days_between(30, 9 - 1, 2017, ofGetDay(), ofGetMonth() - 1, ofGetYear());

		//if (daysTillOpening < -1) {
		//	startDay = 28;
		//	startMonth = 8 - 1;
		//	startYear = 2017;
		//}

		//if (daysTillOpening == -1) {
		//	startDay = 29;
		//	startMonth = 9 - 1;
		//	startYear = 2017;
		//}

		//if (daysTillOpening >= 0) {
		//	startDay = 30;
		//	startMonth = 9 - 1;
		//	startYear = 2017;
		//}

		//endDay = 18;
		//endMonth = 3 - 1;
		//endYear = 2019;
		
		bIsDataLoaded = false;
		bDay = startDay;
		bMonth = startMonth;
		bYear = startYear;

		numTopRiders = 3;
		riderData.topRiderInfo.resize(numTopRiders + 2); // top N + current + last

	}

	bSetBackupPath = false;

	startThread();

}

//--------------------------------------------------------------
void BicycleController::setDefaults() {

	ofLogNotice() << className << ": setDefaults";

	nextSensorMode = SENSOR_SIMULATE;

	wheelDiameter = 678; // in millimetres
	updateVelocityTime = 150; // in millis

	velocityDecay = 1.0;
	velocityEase = 0.5;
	velocityNormalSpeed = 20.0;

	riderInactiveTime = 2000; //millis
	boosterDifficulty = 5;

	numberOfMagnets = 2;
	minimumRiderTime = 10;

#ifdef TARGET_WIN32
	backupPath = "C:/Users/gameover8/Desktop/backup";
#else
	backupPath = "/home/pi/Desktop/backup";
#endif

}

//--------------------------------------------------------------
void BicycleController::update() {

	if (!bUse) return;

	if (bSetBackupPath) {
#ifndef TARGET_WIN32
		ofSetWindowShape(10, 10);
#endif
		ofFileDialogResult result = ofSystemLoadDialog("Select Backup Folder", true);
		if (result.bSuccess) {
			lock();
			backupPath = result.getPath();
			unlock();
		}
#ifndef TARGET_WIN32
		ofSetWindowShape(1920, 1080);
#endif
	}

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
        lastGPIOMsg = "0";
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
				//simulateVelocity = ofRandom(2, 22);
				double wheelCircumference = wheelDiameter * PI;
				double targetVelocity = simulateVelocity * 1000.0 * 1000.0 / 60.0 / 60.0; // km/h * meters * millimeters / minutes / seconds = mm/s
				double simulateTimeout = (wheelCircumference / targetVelocity * 1000.0) / numberOfMagnets; // mm / mm/s * 1000.0 = milliseconds
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

                // if some gpio value, then triggerSensor(SENSOR_GPIO)
                if(gio17_state == "0" && lastGPIOMsg == "1"){
                    triggerSensor(SENSOR_GPIO);
                }
                lastGPIOMsg = gio17_state;
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


							//for (int i = 0; i < 100; i++) {
							//	float rTime = 1 * 60 * 1000;// ofRandom(0.5 * 60 * 1000.0f, 10 * 60 * 1000.0f);
							//	// generate random riders
							//	currentRider = RiderInfo();
							//	currentRider.currentSpeed = 26.0f;// ofRandom(20, 58);
							//	currentRider.distanceTravelled = currentRider.currentSpeed / 60.0 / 60.0 * rTime;
							//	currentRider.day = day;
							//	currentRider.month = month;
							//	currentRider.year = year;
							//	updateRiderInfo();
							//	currentRider.time = rTime; // do this here to avoid overflow in time
							//	todaysRiderInfo.push_back(currentRider);
							//}
							//Serializer.saveClass(ofToDataPath("configs/stats/dailyRiderInfo_" + os.str() + string(CONFIG_TYPE)), (todaysRiderInfo), ARCHIVE_BINARY);
							//Serializer.saveClass(ofToDataPath(backupPath + "/dailyRiderInfo_" + os.str() + string(CONFIG_TYPE)), (todaysRiderInfo), ARCHIVE_BINARY);

							ofLogNotice() << "Loading day: " << os.str();
							Serializer.loadClass(ofToDataPath("configs/stats/dailyRiderInfo_" + os.str() + string(CONFIG_TYPE)), (todaysRiderInfo), ARCHIVE_BINARY);


							float dailyDistance = 0;

							vector<int> deleteIndexes;

							for (int rider = 0; rider < todaysRiderInfo.size(); rider++) {
								if(todaysRiderInfo[rider].topSpeed > 84){
									cout << todaysRiderInfo[rider].topSpeed << "  " << todaysRiderInfo[rider].distanceTravelled << " " << todaysRiderInfo[rider].time << endl;
									deleteIndexes.push_back(rider);
								}else{
									dailyDistance += todaysRiderInfo[rider].distanceTravelled;
									totalTimeTaken += todaysRiderInfo[rider].time;
									totalDistanceTravelled += todaysRiderInfo[rider].distanceTravelled;
									allRiderInfo.push_back(todaysRiderInfo[rider]);
								}
							}
							
							for (int i = deleteIndexes.size() - 1; i >= 0; i--) {
								todaysRiderInfo.erase(todaysRiderInfo.begin() + deleteIndexes[i]);
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

						// sort todays rider info
						vector<RiderInfo>& todaysRiderInfo = getTodaysRiderInfo();
						std::sort(todaysRiderInfo.begin(), todaysRiderInfo.end(), RiderRankFunction);
						std::sort(allRiderInfo.begin(), allRiderInfo.end(), RiderRankFunction);

						int allranking = 0;
						for (int i = 0; i < allRiderInfo.size(); i++) {
							allRiderInfo[i].allranking = allranking;
							if (i < numTopRiders) riderData.topRiderInfo[i] = allRiderInfo[i];
							allranking++;
						}

						if(allRiderInfo.size() > 0) riderData.topRiderInfo[riderData.topRiderInfo.size() - 1] = allRiderInfo[allRiderInfo.size() - 1];

						bIsDataLoaded = true;

						//riderSummary.data = new float[4 + totalDailyDistances.size()];
						riderData.riderSummary.data = new float[RS_DATA_START + activeDaysUsed];
						riderData.riderSummary.data[RS_DATA_SIZE] = RS_DATA_START + activeDaysUsed;
						riderData.riderSummary.data[RS_NUM_DAYS] = activeDaysUsed;

					}
				}
			}

			// ease currentVelocity - both ease toward zero AND ease toward last measured velocity
			if (ofGetElapsedTimeMillis() - lastVelocityTimeout > updateVelocityTime) {
				currentAverageVelocity = CLAMP(currentAverageVelocity * (1.0 - velocityEase) + lastMeasuredVelocity * velocityEase, 0.0f, ofRandom(70, 85));
				currentNormalisedVelocity = currentAverageVelocity / velocityNormalSpeed;
				//lastMeasuredVelocity = lastMeasuredVelocity - velocityDecay; // do we need this? do we do this later? maybe...
				
				// update rider info
				currentRider.currentSpeed = currentAverageVelocity;
				currentRider.normalisedSpeed = currentNormalisedVelocity;
				currentRider.currentKiloWatts = maxWatts[boosterDifficulty] * currentRider.currentSpeed;
				
				if (bRecordRiders && bIsDataLoaded) {

					if (bIsRiderActive) updateRiderInfo();

					riderData.topRiderInfo[riderData.topRiderInfo.size() - 2] = currentRider;

					riderData.riderSummary.data[RS_SPEED_CURRENT] = currentRider.currentSpeed;
					//riderSummary.data[RS_NUM_DAYS] = totalDailyDistances.size();
					riderData.riderSummary.data[RS_IS_ACTIVE] = bIsRiderActive;
					riderData.riderSummary.data[RS_DISTANCE_DAY] = totalDailyDistances[getTodaysRiderIndex()];
					riderData.riderSummary.data[RS_DISTANCE_TOTAL] = totalDistanceTravelled;
					riderData.riderSummary.data[RS_TIME_TOTAL] = (totalTimeTaken + currentRider.time) / 1000.0f;
					riderData.riderSummary.data[RS_RIDERS_TOTAL] = totalNumberRiders;

					int usedDay = 0;

					for (int i = 0; i < totalDailyDistances.size(); i++) {
						if (daysOfWeekToUse[i]) {
							riderData.riderSummary.data[RS_DATA_START + usedDay] = totalDailyDistances[i];
							usedDay++;
						}
					}

				}

				if (lastMeasuredVelocity < 0.0f) lastMeasuredVelocity = 0.0f;
				lastVelocityTimeout = ofGetElapsedTimeMillis();
			}

			// check if rider is inactive
			if (ofGetElapsedTimeMillis() - lastSensorTimeout > riderInactiveTime) {
				
				if (bIsRiderActive) {

					if (lastMeasuredVelocity > 0.0f) {
						// ease the velocity down
						lastMeasuredVelocity = CLAMP(lastMeasuredVelocity - velocityDecay, 0.0f, 120.0f);
					}
					else {
						bIsRiderActive = false;
						currentAverageVelocity = lastMeasuredVelocity = 0.0;


						if (bRecordRiders && bIsDataLoaded && currentRider.time >= minimumRiderTime * 1000) {

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

							int allranking = 0;
							for (int i = 0; i < allRiderInfo.size(); i++) {
								allRiderInfo[i].allranking = allranking;
								if (i < numTopRiders) riderData.topRiderInfo[i] = allRiderInfo[i];
								allranking++;
							}

							riderData.topRiderInfo[riderData.topRiderInfo.size() - 1] = allRiderInfo[allRiderInfo.size() - 1];

							ostringstream os;
							os << "/dailyRiderInfo_" << ofGetYear() << "_" << std::setfill('0') << std::setw(2) << ofGetMonth() << "_" << std::setfill('0') << std::setw(2) << ofGetDay() << string(CONFIG_TYPE);
							Serializer.saveClass(ofToDataPath("configs/stats" + os.str()), (todaysRiderInfo), ARCHIVE_BINARY);
							Serializer.saveClass(ofToDataPath(backupPath + os.str()), (todaysRiderInfo), ARCHIVE_BINARY);
						}
						else {
							currentRider.isActive = false;
							currentRider = RiderInfo();
						}


						ofLogVerbose() << "Rider Inactive";
					}

					
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

	currentRider.topSpeed = MAX(currentRider.topSpeed, currentRider.currentSpeed);
	currentRider.topKiloWatts = MAX(currentRider.topKiloWatts, currentRider.currentKiloWatts);

	currentRider.allranking = 0;

	for (int i = 0; i < allRiderInfo.size(); i++) {
		if (currentRider.topSpeed > allRiderInfo[i].topSpeed) {
			break;
		}
		else {
			currentRider.allranking++;
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
		float dist = (wheelDiameter * PI / 1000.0) / numberOfMagnets;
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
const RiderInfo & BicycleController::getCurrentRiderInfo() {
	ofScopedLock lock(mutex);
	return currentRider;
}

//--------------------------------------------------------------
const RiderData & BicycleController::getRiderData(){
	ofScopedLock lock(mutex);
	return riderData;
}

//--------------------------------------------------------------
bool BicycleController::isDataLoaded() {
	ofScopedLock lock(mutex);
	return bIsDataLoaded;
}

//--------------------------------------------------------------
void BicycleController::drawGUI() {

	if (ImGui::CollapsingHeader(className.c_str())) {
		beginGUI();
		{
			bSetBackupPath = ImGui::Button("Set Backup Path");

			ImGui::SliderInt("Wheel Diameter (mm)", &wheelDiameter, 600, 750);
			ImGui::SliderInt("Number of Magnets", &numberOfMagnets, 1, 4);
			ImGui::SliderInt("Minimum Rider Time (s)", &minimumRiderTime, 1, 30);
			
			ImGui::SliderInt("Update Velocity (millis)", &updateVelocityTime, 20, 1000);

			ImGui::SliderFloat("Velocity Decay (per/update)", &velocityDecay, 0.0, 20.0);
			ImGui::SliderFloat("Velocity Ease (per/update)", &velocityEase, 0.01, 1.0);

			ImGui::SliderInt("Rider Inactive Time (millis)", &riderInactiveTime, 1000, 6000);

			ImGui::SliderFloat("Velocity Normal (km/h)", &velocityNormalSpeed, 0.01, 80.0);

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
				for (int i = 0; i < allRiderInfo.size(); i++) {
					if (numFound < numRequested) {
						ImGui::Text("all: %i  %.3f km/hr  %.3f m  %i / %i / %i", allRiderInfo[i].allranking + 1,
							allRiderInfo[i].topSpeed, allRiderInfo[i].distanceTravelled,
							//getAnimalFromIndex(allRiderInfo[i].currentAnimal).c_str(), getDeviceFromIndex(allRiderInfo[i].currentDevice).c_str(),
							allRiderInfo[i].day, allRiderInfo[i].month, allRiderInfo[i].year);
						numFound++;
					}
				}
				ImGui::NewLine();
				int index = getTodaysRiderIndex();
				ImGui::Text("todays index: %i distance: %.3f m days: %i time: %i used days: %i data: %i bytes", 
					index, totalDailyDistances[getTodaysRiderIndex()], totalDailyDistances.size(), activeDaysUsed, currentRider.time, (4 + activeDaysUsed)*sizeof(float));

			}

			unlock();

		}
		endGUI();
	}
}

//--------------------------------------------------------------
bool BicycleController::loadParameters() {
	cout << "load: " << configPath << endl;
	return Serializer.loadClass(ofToDataPath("configs/" + configPath + "/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}

//--------------------------------------------------------------
bool BicycleController::saveParameters() {
	cout << "save: " << configPath << endl;
	return Serializer.saveClass(ofToDataPath("configs/" + configPath + "/" + className + CONFIG_TYPE), (*this), ARCHIVE_BINARY);
}
