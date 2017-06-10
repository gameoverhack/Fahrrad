#pragma once

#include "ofMain.h"
#include "IGuiBase.h"

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
