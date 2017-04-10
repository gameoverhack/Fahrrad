#pragma once

#include "IGuiBase.h"

#ifdef TARGET_WIN32
// normal OF video player
#else
#include "ofxOMXVideo.h"
#endif

class VideoController : public IGuiBase {

public:

	VideoController();
	~VideoController();

	void setup();
	void update();
	void drawGUI();

	void setSpeed(float speed);

	ofTexture& getVideoTexture();

protected:

#ifdef TARGET_WIN32
	ofVideoPlayer vid;
#else
	ofxOMXVideo vid;
#endif

	ofDirectory dir;
	string videoPath;
	vector<string> videoFilePaths;

	int lastSpeedUpdateTime;
	int speedUpdateTimeout;

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
	}

};

