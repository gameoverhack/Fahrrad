#pragma once

#include "ofMain.h"
#include "IGuiBase.h"
#include "ofxHomography.h"

class RenderController : public IGuiBase{
public:

	RenderController();
	~RenderController();

	void setup(string path) {
		configPath = path;
		setup();
	}

	void setup();
	void update();
	void draw();
	void drawGUI();

	void begin();
	void end();

	void mouseDragged(int x, int y);
	void mousePressed(int x, int y);
	void mouseReleased(int x, int y);

protected:

	ofFbo fbo;
	
	bool bSetDistortion;

	ofPoint originalCorners[4];
	ofPoint distortedCorners[4];
	ofMatrix4x4 homography;

	int currentPointIndex;

	void setDefaults();
	bool loadParameters();
	bool saveParameters();

	friend class boost::serialization::access;
	
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_NVP(distortedCorners);
	}
	
};
