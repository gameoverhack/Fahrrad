#pragma once

#include "ofMain.h"
#include "IGuiBase.h"
#include "ofxFlickr.h"

class ImageCaptureController : public IGuiBase, public ofThread {
public:

	ImageCaptureController();
	~ImageCaptureController();

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
		PHOTO_TAKEIMAGE,
		PHOTO_SAVEIMAGE,
		PHOTO_FINISHING,
		PHOTO_FINISHED
	} PhotoState;

	void setup();
	void update();
	void drawGUI();

	void triggerSensor(SensorMode sensorMode);
	
	ofTexture& getCameraTexture();

protected:

	ofxFlickr::API flickr;
	
	int flickrAuthenticateTimeout;
	int lastFlickrAuthenticateTime;

	void onFlickrEvent(ofxFlickr::APIEvent & evt);

	bool bSetImageStorePath;
	string imageStorePath;

	string currentImageFilePath;
	vector<string> uploadQueue;

	ofVideoGrabber cam;

	float brightness, contrast, saturation;

	ofShader shader;
	ofMesh mesh;

	ofFbo fbo;
	ofPixels pixels;

	vector<string> photoStates = {
		"PHOTO_NONE",
		"PHOTO_REQUESTED",
		"PHOTO_COUNTDOWN",
		"PHOTO_TAKEIMAGE",
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
	int simulateTimeout;

	////Audio
	ofSoundPlayer soundPlayerCountdown;
	ofSoundPlayer soundPlayerShutter;

	void playCountdownSound();
	void playShutterSound();

	void takeImage();
	void saveImage();

	void changeMode();

	void threadedFunction();

	void setDefaults();
	bool loadParameters();
	bool saveParameters();

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_NVP(nextSensorMode);
		ar & BOOST_SERIALIZATION_NVP(imageStorePath);
		ar & BOOST_SERIALIZATION_NVP(brightness);
		ar & BOOST_SERIALIZATION_NVP(contrast);
		ar & BOOST_SERIALIZATION_NVP(saturation);
		ar & BOOST_SERIALIZATION_NVP(simulateTimeout);
		ar & BOOST_SERIALIZATION_NVP(flickrAuthenticateTimeout);
		ar & BOOST_SERIALIZATION_NVP(uploadQueue);
	}
};
