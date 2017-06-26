#pragma once

#include "ofMain.h"
#include "IGuiBase.h"

#ifndef TARGET_WIN32
#include "ofxGPIO.h"
#endif

class PulseController : public IGuiBase, public ofThread {

public:

	PulseController();
	~PulseController();

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

	const PulseData& getPulseData();

protected:

	ofSerial serial;
	PulseDataUnion pdu;

	bool bIsPulseActive;

	int targetPulseRate;
	int pulseRecvDelay;

	vector<string> sensorModes = {
		"SENSOR_NONE",
		"SENSOR_SIMULAT",
		"SENSOR_KEYBOARD",
		"SENSOR_TEENSY",
		"SENSOR_GPIO"
	};

	SensorMode nextSensorMode;
	SensorMode currentSensorMode;

	double timeSinceLastSensor;
	int lastSensorTimeout;

	//void calculateBPM();

	//int sampleCounter = 0;
	//int timeSinceLastSignal = 0;
	//int timeAtLastSignal = 0;
	//int lastBeatTime = 0;

	//bool Pulse = false;
	//bool QS = false;
	//bool firstBeat = true;
	//bool secondBeat = false;
	//int BPM = 0;
	//int IBI = 600;
	//int amp = 0;
	//int thresh = 530;
	//int P = 512;
	//int T = 512;
	//int rate[10];

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
		ar & BOOST_SERIALIZATION_NVP(targetPulseRate);
		ar & BOOST_SERIALIZATION_NVP(pulseRecvDelay);
	}

};

