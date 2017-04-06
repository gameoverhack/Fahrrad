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

	double getAverageVelocity();

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
	int updateVelocityTime;
	int lastVelocityTimeout;

	int simulateVelocity;

	double lastMeasuredVelocity;
	double currentAverageVelocity;

	double timeSinceLastSensor;
	int lastSensorTimeout;

	//string getMode();

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
		ar & BOOST_SERIALIZATION_NVP(updateVelocityTime);
		ar & BOOST_SERIALIZATION_NVP(simulateVelocity);
	}

};

