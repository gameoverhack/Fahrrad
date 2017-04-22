#pragma once

#include "ofMain.h"
#include "IGuiBase.h"

class CamController : public IGuiBase, public ofThread {
public:

	CamController();
	~CamController();

	typedef enum {
		SENSOR_NONE = 0,
		SENSOR_SIMULATE,
		SENSOR_KEYBOARD,
		SENSOR_TEENSY,
		SENSOR_GPIO
	} SensorMode;

	typedef enum {
		PHOTO_NONE = 0,
		PHOTO_REQUESTED,
		PHOTO_COUNTDOWN,
		PHOTO_SAVEIMAGE,
		PHOTO_FINISHING,
		PHOTO_FINISHED
	} PhotoState;

	void setup();
	void update();
	void drawGUI();

	void triggerSensor(SensorMode sensorMode);
	void saveIMG();

	ofTexture& getCameraTexture();

protected:

	float brightness, contrast, saturation;

	bool bSetImageStorePath;

	string imgStorePath;

	ofVideoGrabber cam;
	ofImage ofSaveIMG;
	ofShader shader;
	ofFbo fbo;
	ofMesh mesh;
	ofPixels pixels;

	vector<string> photoStates = {
		"PHOTO_NONE",
		"PHOTO_REQUESTED",
		"PHOTO_COUNTDOWN",
		"PHOTO_SAVEIMAGE",
		"PHOTO_FINISHING",
		"PHOTO_FINISHED"
	};

	vector<string> sensorModes = {
		"SENSOR_NONE",
		"SENSOR_SIMULAT",
		"SENSOR_KEYBOARD",
		"SENSOR_TEENSY",
		"SENSOR_GPIO"
	};

	PhotoState currentPhotoState;

	SensorMode nextSensorMode;
	SensorMode currentSensorMode;

	double timeSinceLastSensor;
	int lastSensorTimeout;

	////Audio
	ofSoundPlayer soundPlayerCountdown;
	ofSoundPlayer soundPlayerShutter;

	void playCountdownSound();
	void playShutterSound();


	void changeMode();

	void threadedFunction();

	void setDefaults();
	bool loadParameters();
	bool saveParameters();

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_NVP(nextSensorMode);
		// save image vars
		ar & BOOST_SERIALIZATION_NVP(imgStorePath);
		ar & BOOST_SERIALIZATION_NVP(brightness);
		ar & BOOST_SERIALIZATION_NVP(contrast);
		ar & BOOST_SERIALIZATION_NVP(saturation);
	}
};
