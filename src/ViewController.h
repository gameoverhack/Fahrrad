#pragma once

#include "ofMain.h"
#include "IGuiBase.h"
#include "ofxSvg.h"
#include "ofxTextAlignTTF.h"
#include "ofxXmlSettings.h"

class ViewController : public IGuiBase, public ofThread {
public:

	ViewController();
	~ViewController();

	typedef enum {
		VIEW_NONE = 0,
		VIEW_SEND,
		VIEW_RECV
	} ViewMode;

	void setup();
	void setMode(ViewMode mode);
	void update();
	void drawGUI();

	void setData(const RiderSummaryUnion& rsu, const vector<RiderInfo>& tri = vector<RiderInfo>());
	const ofFbo& getFBO();

protected:

	ofFbo fbo;
	ofFbo backgroundFbo;

	// SENDER FBOs
	ofFbo dialRedFbo;
	ofFbo dialMaxFbo;
	ofFbo dialGreyFbo;
	ofFbo dialMaskFbo;

	// SENDER FONTS
	ofTrueTypeFont fDeviceBold;
	ofTrueTypeFont fDeviceItalic;
	ofxTextAlignTTF fWattsCurrent;
	ofxTextAlignTTF fSpeedCurrent;
	ofxTextAlignTTF fSpeedHigh;
	ofxTextAlignTTF fDistanceTime;
	ofxTextAlignTTF fHeartRate;
	ofxTextAlignTTF fHighScores;

	// RECEIVER POLY/SVG
	ofPolyline polyDEOutlines;

	// RECEIVER FONTS
	ofTrueTypeFont fAnimalBold;
	ofTrueTypeFont fAnimalItalic;
	ofxTextAlignTTF fPercentageDone;
	ofxTextAlignTTF fSpeedCurrent2;
	ofxTextAlignTTF fDistanceTime2;

	typedef struct {
		float value;
		string type;
	} MileStone;

	vector<MileStone> milestonesSpeed;
	vector<MileStone> milestonesWatts;

	bool bViewNeedsUpdate;
	RiderSummaryUnion riderSummary;
	vector<RiderInfo> topRiderInfo;

	double lastViewTimeout;
	int viewTimeout;

	vector<string> viewModes = {
		"VIEW_NONE",
		"VIEW_SEND",
		"VIEW_RECV"
	};

	ViewMode nextViewMode;
	ViewMode currentViewMode;

	void renderSender();
	void renderReciever();

	void renderSvgToFbo(string filePath, ofFbo& svgFbo, float w, float h, ofColor c);
	int getTodaysRiderIndex();

	void changeMode();

	void threadedFunction();

	void setDefaults();
	bool loadParameters();
	bool saveParameters();

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & BOOST_SERIALIZATION_NVP(viewTimeout);
	}
};
