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

	void setup();
	void update();
	void drawGUI();

	void setSpeed(float speed);

	const float& getVideoFadeThreshold();
	const ofTexture& getVideoTexture();

	void rewind();

	int getFrame();

protected:

#ifdef TARGET_WIN32
	ofVideoPlayer vid;
#else
	ofxOMXPlayer vid;
	ofTexture defaultTexture;
#endif
	
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
	}

};

