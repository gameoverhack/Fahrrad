#include "BicycleController.h"

bool RiderRankFunction(const RiderInfo& a, const RiderInfo& b) { 
	return a.topSpeed > b.topSpeed;
}

vector<string> daysOfTheWeek = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

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
	distanceTravelled = 0; // meters

	bIsRiderActive = false;

	if (bRecordRiders) {
		
		totalDistanceTravelled = 0;




		// we need to generate the dates for all days in the week
		// we are loading from Monday to Monday of whatever month, year, day

		// get the current weekday - this tells us how many days prior and how many days future we need to load

		int weekDay = ofGetWeekday();			// 0 == Sunday, 1 == Monday...
		weekDay = ofWrap(weekDay - 1, 0, 7);	// 6 == Sunday, 0 == Monday, 1 == Tuesday, 2 == Wednesday, 3 == Thursday, 4 == Friday, 5 == Saturday, 6 == Sunday

		// weekDay should give us the number of days between the current 
		// day and Monday eg., when Monday: 0; when Thursday = 3 etc

		int todaysDay = ofGetDay();
		int todaysMonth = ofGetMonth();
		int todaysYear = ofGetYear();
		int thisDay = todaysDay;
		int thisMonth = todaysMonth;
		int thisYear = todaysYear;
		
		// push all previous days (when 0 this doesn't execute
		int i = 0;
		for (; i < weekDay; i++) {
			// push todays date (actually previous day)
			datesToLoad.push_back(date_as_string(thisDay, thisMonth, thisYear));
			ofLogNotice() << i << " == " << daysOfTheWeek[weekDay - i] << datesToLoad[datesToLoad.size() - 1] << endl;

			thisDay--;
			if (thisDay == 0) { // then we are at the beginning/end of a month
				if (todaysMonth - 1 == 0) { // then we are at the beginning/end of a year
					thisMonth = 12;
					thisYear--;
				}
				else {
					thisMonth--;
				}
				thisDay = days_in_month(thisMonth, thisYear);
			}
		}
		
		
		// push todays date (actually today)
		datesToLoad.push_back(date_as_string(thisDay, thisMonth, thisYear));
		ofLogNotice() << i << " == " << daysOfTheWeek[weekDay - i] << datesToLoad[datesToLoad.size() - 1] << endl;


		bIsDataLoaded = false;

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

	velocityModifier = 1.0f;
	velocityMaximum = 60.0f;

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

					

					for (int i = 0; i < datesToLoad.size(); i++) {

						vector<RiderInfo> todaysRiderInfo;

						string date = datesToLoad[i];

						ofLogNotice() << "Loading day: " << date;
						Serializer.loadClass(ofToDataPath("configs/stats/dailyRiderInfo_" + date + string(CONFIG_TYPE)), (todaysRiderInfo), ARCHIVE_BINARY);

						float dailyDistance = 0;

						vector<int> deleteIndexes;

						for (int rider = 0; rider < todaysRiderInfo.size(); rider++) {
							if (todaysRiderInfo[rider].topSpeed > velocityMaximum) {
								cout << todaysRiderInfo[rider].topSpeed << "  " << todaysRiderInfo[rider].distanceTravelled << " " << todaysRiderInfo[rider].time << endl;
								deleteIndexes.push_back(rider);
							}
							else {
								dailyDistance += todaysRiderInfo[rider].distanceTravelled;
								totalTimeTaken += todaysRiderInfo[rider].time;
								totalDistanceTravelled += todaysRiderInfo[rider].distanceTravelled;
								allRiderInfo.push_back(todaysRiderInfo[rider]);
							}
						}

						for (int i = deleteIndexes.size() - 1; i >= 0; i--) {
							todaysRiderInfo.erase(todaysRiderInfo.begin() + deleteIndexes[i]);
						}

						vector<string> dateSplit = ofSplitString(date, "_");
						int dayOfWeek = day_of_week(ofToInt(dateSplit[2]), ofToInt(dateSplit[1]), ofToInt(dateSplit[0]));
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

						totalNumberRiders += todaysRiderInfo.size();
						totalDailyDistances.push_back(dailyDistance);

						dailyRiderInfo[date] = todaysRiderInfo;

					}
					
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

					if (allRiderInfo.size() > 0) riderData.topRiderInfo[riderData.topRiderInfo.size() - 1] = allRiderInfo[allRiderInfo.size() - 1];

					//riderSummary.data = new float[4 + totalDailyDistances.size()];
					riderData.riderSummary.data = new float[RS_DATA_START + activeDaysUsed];
					riderData.riderSummary.data[RS_DATA_SIZE] = RS_DATA_START + activeDaysUsed;
					riderData.riderSummary.data[RS_NUM_DAYS] = activeDaysUsed;

					bIsDataLoaded = true;

				}
			}

			// ease currentVelocity - both ease toward zero AND ease toward last measured velocity
			if (ofGetElapsedTimeMillis() - lastVelocityTimeout > updateVelocityTime) {
				currentAverageVelocity = CLAMP(currentAverageVelocity * (1.0 - velocityEase) + lastMeasuredVelocity * velocityEase, 0.0f, velocityMaximum);
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
					riderData.riderSummary.data[RS_MAX_VELOCITY] = velocityMaximum;

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
	return datesToLoad.size() - 1;// days_between(startDay, startMonth, startYear, ofGetDay(), ofGetMonth() - 1, ofGetYear());
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
		lastMeasuredVelocity = (dist / 1000.0) / (timeSinceLastSensor / 1000.0 / 60.0 / 60.0) * velocityModifier;
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

			ImGui::SliderFloat("Velocity Modifier", &velocityModifier, 0.5, 1.5);
			ImGui::SliderFloat("Velocity Maximum", &velocityMaximum, 50, 80);
			
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
