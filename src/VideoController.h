#pragma once

#include "IGuiBase.h"

#ifdef TARGET_WIN32
// normal OF video player
#else
#include "ofxOMXPlayer.h"
#endif

class VideoController : public IGuiBase {

public:

	VideoController();
	~VideoController();

	void setup(string path) {
		configPath = path;
		setup();
	}

	void setup();
	void update();
	void drawGUI();

	void setSpeed(float speed);

	const float& getVideoFadeThreshold();
	const ofTexture& getVideoTexture();

	void fastforward();
	void rewind();
	int isLooping();

protected:

#ifdef TARGET_WIN32
	ofVideoPlayer vid;
#else
	ofxOMXPlayer vid;
	ofTexture defaultTexture;
#endif
	
	int startLoopFrame;
	int endLoopFrame;
	int pauseFrame;

	bool bRewindPending;

	ofDirectory dir;
	string videoPath;
	vector<string> videoFilePaths;

	int lastSpeedUpdateTime;
	int speedUpdateTimeout;

	float videoFadeThreshold;

	int nextVideoIndex;
	int currentVideoIndex;

	bool bSetVideoPath;

	void listDirectory();

	void setDefaults();
	bool loadParameters();
	bool saveParameters();

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_NVP(videoPath);
		ar & BOOST_SERIALIZATION_NVP(currentVideoIndex);
		ar & BOOST_SERIALIZATION_NVP(speedUpdateTimeout);
		ar & BOOST_SERIALIZATION_NVP(videoFadeThreshold);
		ar & BOOST_SERIALIZATION_NVP(pauseFrame);
		ar & BOOST_SERIALIZATION_NVP(startLoopFrame);
		ar & BOOST_SERIALIZATION_NVP(endLoopFrame);
	}

};

