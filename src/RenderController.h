#pragma once

#include "ofMain.h"
#include "IGuiBase.h"
#include "ofxHomography\src\ofxHomography.h"
#include "ImageLoadController.h"

class RenderController : public IGuiBase, public ofThread {
public:

	RenderController();
	~RenderController();

	void setup();
	void update();
	void drawGUI();

	void mouseDragged(int x, int y);
	void mousePressed(int x, int y);
	void mouseReleased(int x, int y);

	ofTexture& getTexture();
	ImageLoadController imageLoadController;

protected:

	ofFbo fbo;
	
	bool bSetDistortion;
	ofPoint v_0, v_1, v_2, v_3;
	vector<ofPoint> distortionPoints;
	ofPoint originalCorners[4];
	ofPoint distortedCorners[4];
	ofMatrix4x4 homography;

	bool movingPoint;
	ofPoint* curPoint;

	void setDefaults();
	bool loadParameters();
	bool saveParameters();

	friend class boost::serialization::access;
	
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		// distortion vars
		ar & BOOST_SERIALIZATION_NVP(v_0);
		ar & BOOST_SERIALIZATION_NVP(v_1);
		ar & BOOST_SERIALIZATION_NVP(v_2);
		ar & BOOST_SERIALIZATION_NVP(v_3);
	}
	
};
