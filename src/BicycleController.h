#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "IGuiBase.h"

#ifndef TARGET_WIN32
#include "ofxGPIO.h"
#endif

class BicycleController : public IGuiBase, public ofThread {

public:

	BicycleController();
	~BicycleController();

	typedef enum {
		SENSOR_NONE = 0,
		SENSOR_SIMULATE,
		SENSOR_KEYBOARD,
		SENSOR_TEENSY,
		SENSOR_GPIO
	} SensorMode;

	void setup();
	void update();
	void drawGUI();

	void triggerSensor(SensorMode sensorMode);
	void setRecordRiders(bool b);
	const RiderInfo& getCurrentRiderInfo();
	bool isDataLoaded();

	string getAnimalFromIndex(const int& index);
	string getDeviceFromIndex(const int& index);

protected:

	int startDay;
	int startMonth;
	int startYear;

	int endDay;
	int endMonth;
	int endYear;

	int bDay;
	int bMonth;
	int bYear;

	bool bIsDataLoaded;

	float totalDistanceTravelled;

	vector<RiderInfo>& getTodaysRiderInfo();
	void updateRiderInfo();

	typedef struct {
		float value;
		string type;
	} MileStone;

	int boosterDifficulty;
	vector<float> maxWatts = { 285.0f / 60.0f, 370.0f / 60.0f, 455.0f / 60.0f, 540.0f / 60.0f, 625.0f / 60.0f, 710.0f / 60.0f, 795.0f / 60.0f, 880.0f / 60.0f, 965.0f / 60.0f, 1050.0f / 60.0f };

	vector<MileStone> milestonesSpeed;
	vector<MileStone> milestonesWatts;

	vector<RiderInfo> allRiderInfo;
	unordered_map< string, vector<RiderInfo> > monthlyRiderInfo;

	//vector<RiderInfo> allRiderInfo;
	RiderInfo currentRider;

	int riderStartTimeMillis;

	bool bRecordRiders;

	vector<string> sensorModes = {
		"SENSOR_NONE",
		"SENSOR_SIMULAT",
		"SENSOR_KEYBOARD",
		"SENSOR_TEENSY",
		"SENSOR_GPIO"
	};

	SensorMode nextSensorMode;
	SensorMode currentSensorMode;

	int wheelDiameter;

	float velocityDecay;
	float velocityEase;
	float velocityNormalSpeed;
	double currentNormalisedVelocity;
	int updateVelocityTime;
	int lastVelocityTimeout;

	bool bIsRiderActive;
	int riderInactiveTime;

	double distanceTravelled;

	int simulateVelocity;

	double lastMeasuredVelocity;
	double currentAverageVelocity;

	double timeSinceLastSensor;
	int lastSensorTimeout;

#ifndef TARGET_WIN32
    GPIO gpio17;
    string gio17_state;
    string lastMsg;
#endif

	void changeMode();

	void threadedFunction();

	void setDefaults();
	bool loadParameters();
	bool saveParameters();

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_NVP(nextSensorMode);
		ar & BOOST_SERIALIZATION_NVP(wheelDiameter);
		ar & BOOST_SERIALIZATION_NVP(velocityDecay);
		ar & BOOST_SERIALIZATION_NVP(velocityEase);
		ar & BOOST_SERIALIZATION_NVP(velocityNormalSpeed);
		ar & BOOST_SERIALIZATION_NVP(updateVelocityTime);
		ar & BOOST_SERIALIZATION_NVP(riderInactiveTime);
		ar & BOOST_SERIALIZATION_NVP(boosterDifficulty);
	}

};

