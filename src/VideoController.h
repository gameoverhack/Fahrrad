#pragma once

#include "IGuiBase.h"

class VideoController : public IGuiBase {

public:

	VideoController();
	~VideoController();

	void setup();
	void update();
	void drawGUI();

	ofTexture& getVideoTexture();

protected:

	ofVideoPlayer vid;

	ofDirectory dir;
	string videoPath;
	vector<string> videoFilePaths;

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
	}

};

