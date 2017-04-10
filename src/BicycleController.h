#pragma once

#include "ofMain.h"
#include "IGuiBase.h"

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

	bool getIsRiderActive();
	double getAverageVelocity();   // km/hour
	double getNormalisedVelocity(); // 0.0 -> 1.0 -> x.0
	double getDistanceTravelled(); // metres

protected:

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
	}

};

