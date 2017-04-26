#pragma once

#include "ofMain.h"
#include "IGuiBase.h"

class ImageLoadController : public IGuiBase, public ofThread {
public:

	ImageLoadController();
	~ImageLoadController();

	void setup();
	void update();
	void draw();
	void drawGUI();

protected:
	void threadedFunction();

	bool bSetImageLoadPath;

	ofDirectory dir;
	string tmpImage;
	string imgLoadPath;
	vector<string> imgFilePaths;
	
	//Timer
	float startTime;
	float curTime;

	//Buffer Images
	vector<ofFbo> fbos;
	vector<ofImage> images;
	
	int currentFboIndex;

	bool bSetupImages;
	bool bLoadImages;
	bool bImagesLoaded;
	bool bImagesReady;

	void loadImages();

	float transitionTime;

	void setDefaults();
	bool loadParameters();
	bool saveParameters();

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		// load image vars
		ar & BOOST_SERIALIZATION_NVP(imgLoadPath);	
	}
};