#pragma once

#include "ofMain.h"
#include "IGuiBase.h"

//#ifndef TARGET_WIN32
//#include "ofxGPIO.h"
//#endif

class PulseController : public IGuiBase, public ofThread {

public:

	PulseController();
	~PulseController();

	typedef enum {
		SENSOR_NONE = 0,
		SENSOR_SIMULATE,
		SENSOR_ARDUINO,
	} SensorMode;

	void setup(string path) {
		configPath = path;
		setup();
	}

	void setup();
	void update();
	void drawGUI();

	vector<PulseData> getPulseData();
	vector<PulseData> getRawPulseData();

protected:

	ofSerial serial;
	PulseDataUnion pdu;

	bool bIsPulseActive;

	int targetPulseRate;
	int sensorReadTimeout;
	int sensorUpdateTimeout;

	int lastUpdateTimeout;
	int lastSensorTimeout;

	vector<string> sensorModes = {
		"SENSOR_NONE",
		"SENSOR_SIMULAT",
		"SENSOR_ARDUINO",
	};

	SensorMode nextSensorMode;
	SensorMode currentSensorMode;

	vector<PulseData> pulseData;
	int currentPulseDataIndex;

	vector<PulseData> rawPulseData;
	int currentRawPulseDataIndex;

	void changeMode();

	void threadedFunction();

	void setDefaults();
	bool loadParameters();
	bool saveParameters();

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_NVP(nextSensorMode);
		ar & BOOST_SERIALIZATION_NVP(targetPulseRate);
		ar & BOOST_SERIALIZATION_NVP(sensorReadTimeout);
		ar & BOOST_SERIALIZATION_NVP(sensorUpdateTimeout);
	}

};

